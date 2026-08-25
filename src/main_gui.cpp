// main_gui.cpp
// GUI front-end for the flashcard app, built with Dear ImGui + SDL2 + OpenGL3.
//
// This file replaces the terminal main.cpp's input loop with an ImGui-driven
// event loop. All the actual flashcard logic (Card, LearnedTracker, fuzzy
// matching, logResult) is untouched and reused directly from Cards.h/.cpp.

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengl.h>

#include <vector>
#include <string>
#include <random>
#include <cstring>
#include "Cards.h"

using namespace std;

// ---------------------------------------------------------------------------
// Application state — this is the GUI equivalent of the local variables that
// used to live inside main()'s do/while and while loops in the terminal app.
// ---------------------------------------------------------------------------
struct AppState
{
    vector<Card> deck;
    int idCt = 0;              // number of "active" (not-yet-mastered) cards
    mt19937 gen{ random_device{}() };

    int round = 1;
    int currentIndex = -1;     // index into deck of the card being shown
    bool answered = false;     // true once the user has submitted an answer
    bool wasCorrect = false;
    string feedbackMsg;
    char answerBuf[256] = "";
    bool sessionComplete = false;

    // Buffers for the "add a card at runtime" panel.
    char newFrontBuf[256] = "";
    char newBackBuf[256] = "";
};

// Build the initial deck. Same cards as the terminal version's main.cpp.
static vector<Card> makeDeck()
{
    return
    {
        {"Who won the 2026 World Cup?", "Brazil"},
        {"What part of the brain controls language production?", "Broca's"},
        {"What part of the brain controls language comprehension?", "Wernicke's"},
        {"What is your cat's name?", "Qiqi"},
        {"What is Jay's favorite color?", "Orange"}
    };
}

// Picks a new random active card, or flags the session complete if none remain.
// This mirrors the `uniform_int_distribution<int> dist(0, idCt-1)` logic from
// the original while(idCt > 0) loop in main.cpp.
static void pickNextCard(AppState& s)
{
    if (s.idCt <= 0)
    {
        s.sessionComplete = true;
        s.currentIndex = -1;
        return;
    }
    uniform_int_distribution<int> dist(0, s.idCt - 1);
    s.currentIndex = dist(s.gen);
    s.answered = false;
    s.wasCorrect = false;
    s.feedbackMsg.clear();
    s.answerBuf[0] = '\0';
}

// Resets trackers and starts a fresh round. Mirrors the top of the
// do/while(choice == 'y') loop in the original main().
static void restartSession(AppState& s)
{
    for (Card& card : s.deck)
        card.resetTracker();

    s.idCt = (int)s.deck.size();
    s.round = 1;
    s.sessionComplete = false;
    pickNextCard(s);
}

// Handles the "submit answer" action. Mirrors the body of the while(idCt > 0)
// loop after getline() in the terminal version.
static void submitAnswer(AppState& s)
{
    if (s.currentIndex < 0 || s.answered)
        return;

    Card& currCard = s.deck[s.currentIndex];
    string answer(s.answerBuf);

    if (currCard == answer)
    {
        s.wasCorrect = true;
        s.feedbackMsg = "Correct!";
        logResult(true, currCard);

        if (currCard.tracker.isLearned())
        {
            s.feedbackMsg += "  Card mastered and removed from the deck!";
            swap(s.deck[s.currentIndex], s.deck[s.idCt - 1]);
            s.idCt--;
        }
    }
    else
    {
        s.wasCorrect = false;
        s.feedbackMsg = "Incorrect. The correct answer is: " + currCard.back;
        logResult(false, currCard);
    }

    s.round++;
    s.answered = true;
}

// Adds a newly-created card into the *active* part of the deck.
//
// Recall the deck invariant used throughout this app (and the original
// terminal version): deck[0 .. idCt) are the still-active cards for this
// round, and deck[idCt .. size()) are cards already mastered this round.
// A brand-new card should join the active side, so we push it to the back
// of the whole vector, then swap it into position idCt (bumping whatever
// mastered card was sitting there out to the new end) and grow idCt by one.
static void addCard(AppState& s, const string& term, const string& def)
{
    if (term.empty() || def.empty())
        return;

    Card c = newCard(term, def);
    s.deck.push_back(c);

    size_t justAdded = s.deck.size() - 1;
    swap(s.deck[justAdded], s.deck[s.idCt]);
    s.idCt++;

    // If the session had already been marked complete (deck was empty of
    // active cards), this new card means there's something to study again.
    if (s.sessionComplete)
    {
        s.sessionComplete = false;
        pickNextCard(s);
    }
}

// ---------------------------------------------------------------------------
// The actual UI. Called once per frame from the render loop below.
// ---------------------------------------------------------------------------
static void RenderFlashcardUI(AppState& s)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Flashcards", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    ImGui::SetWindowFontScale(1.4f);

    if (s.sessionComplete)
    {
        ImGui::Dummy(ImVec2(0, 40));
        ImGui::TextWrapped("Session complete! All cards answered correctly.");
        ImGui::Dummy(ImVec2(0, 20));
        if (ImGui::Button("Restart session", ImVec2(200, 50)))
            restartSession(s);
        ImGui::SameLine();
        if (ImGui::Button("Quit", ImVec2(120, 50)))
        {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }
    }
    else if (s.currentIndex >= 0)
    {
        Card& currCard = s.deck[s.currentIndex];

        ImGui::Text("Question #%d", s.round);
        ImGui::Text("Cards remaining this round: %d", s.idCt);
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));

        ImGui::TextWrapped("%s", currCard.front.c_str());
        ImGui::Dummy(ImVec2(0, 20));

        ImGui::SetNextItemWidth(400);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        bool enterPressed = ImGui::InputText("##answer", s.answerBuf, sizeof(s.answerBuf), flags);

        // Keep keyboard focus on the input box so typing feels like the terminal app.
        if (ImGui::IsWindowAppearing() || (!s.answered && !ImGui::IsAnyItemActive()))
            ImGui::SetKeyboardFocusHere(-1);

        ImGui::SameLine();
        bool submitClicked = ImGui::Button("Submit");

        if (!s.answered && (enterPressed || submitClicked))
            submitAnswer(s);

        if (s.answered)
        {
            ImGui::Dummy(ImVec2(0, 10));
            ImVec4 color = s.wasCorrect ? ImVec4(0.3f, 0.9f, 0.3f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
            ImGui::TextColored(color, "%s", s.feedbackMsg.c_str());
            ImGui::Dummy(ImVec2(0, 10));

            if (ImGui::Button("Next card", ImVec2(150, 45)))
                pickNextCard(s);
        }
    }

    // --- Add a card at runtime ---------------------------------------------
    ImGui::Dummy(ImVec2(0, 30));
    ImGui::Separator();
    ImGui::SetWindowFontScale(1.1f);

    if (ImGui::CollapsingHeader("Add a card"))
    {
        ImGui::SetNextItemWidth(400);
        ImGui::InputText("Term / Question", s.newFrontBuf, sizeof(s.newFrontBuf));
        ImGui::SetNextItemWidth(400);
        bool enterInBack = ImGui::InputText("Definition / Answer", s.newBackBuf, sizeof(s.newBackBuf),
                                             ImGuiInputTextFlags_EnterReturnsTrue);

        bool addClicked = ImGui::Button("Add card");
        bool canAdd = s.newFrontBuf[0] != '\0' && s.newBackBuf[0] != '\0';

        if (!canAdd)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("(enter both fields)");
        }

        if (canAdd && (addClicked || enterInBack))
        {
            addCard(s, s.newFrontBuf, s.newBackBuf);
            s.newFrontBuf[0] = '\0';
            s.newBackBuf[0] = '\0';
        }

        ImGui::TextDisabled("Total cards in deck: %d", (int)s.deck.size());
    }

    ImGui::End();
}

// ---------------------------------------------------------------------------
// Boilerplate below is the standard Dear ImGui "SDL2 + OpenGL3" application
// shell (init window/context, run event+render loop, teardown). You generally
// don't need to touch this part when adding new screens/widgets — you only
// edit RenderFlashcardUI above.
// ---------------------------------------------------------------------------
int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Flashcards", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 500, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    AppState state;
    state.deck = makeDeck();
    restartSession(state);

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderFlashcardUI(state);

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.11f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
