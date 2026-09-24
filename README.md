Flashcards GUI

A desktop flashcard app built in C++ featuring Dear ImGui, SDL2 and openGL

Originally a terminal-based flashcard program utilizing retrival and repetition to promote higher flashcard retention and long term memory.

Features
- flashcards with front/back (tern/answer) that allows up to 2 minor typos but accounts for capitalization missmatches regardless
- cards answered correctly 3 times in a row are considered "mastered" and are removed from the current round
- cards will appear randomly from the pool of total flashcards, only being temporarily removed when they become "mastered"
- create and remove flashcards at runtime through the GUI
- deck is saved to disk automatically and reloaded upon next startup

Requirements
- C++17 compiler
- SDL2 headers
- OpenGL

1. Install prerequisites
    git, C++ compiler, Homebrew (Mac only from brew.sh)
2. Clone the repository
    run "git clone (https://github.com/chillychinchila/Flashcards.git) and proceed to "cd flashcards-gui" to move into the project folder
3. Instal SDL2
    macOS: 'brew install sdl2' and run 'sdl2-config --version' to verify sld2 install
4. Run the app
    run 'make' from inside the project folder and './flashcards_gui' to open app

Possible future improvements
- better color scheme in app
- editing existing cards without deleting/creating new 
- multiple decks and/or categories
- packaged installers for macOS and Windows