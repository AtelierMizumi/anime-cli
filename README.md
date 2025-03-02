# 📺 Anime CLI

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Lines of code](https://img.shields.io/badge/lines%20of%20code-1.5k%2B-brightgreen.svg)
![Language](https://img.shields.io/badge/language-C-orange.svg)
![AUR](https://img.shields.io/badge/AUR-pending-yellow.svg)
![Dependencies](https://img.shields.io/badge/dependencies-curl%20|%20json--c%20|%20ncurses%20|%20mpv-informational)

A terminal-based anime streaming client that allows you to search, browse, and watch anime directly from your terminal using the Consumet API.

This project is primary used to test out the Consumet self-host API which will then be used for the [Kotsune Android App](https://github.com/AtelierMizumi/Kotsune)

## 📋 Table of Contents

- Features
- Requirements
- Installation
  - From Source
  - From AUR (Arch User Repository)
- Usage
  - Basic Commands
  - Keyboard Shortcuts
- Screenshots
- Development
- Contributing
- License
- Acknowledgements

## ✨ Features

- 🔍 **Search anime** - Find any anime with simple search queries
- 📺 **Stream episodes** - Watch anime episodes directly from your terminal
- 🌐 **Multi-provider support** - Uses the Consumet API for reliable access to content (soonTM)
- 📋 **Episode tracking** - Easy episode selection interface
- 📱 **Lightweight design** - Minimal resource usage
- 🔤 **Multi-language subtitles** - Built-in subtitle selection support (very soonTM)
- ⌨️ **Keyboard navigation** - Fast, intuitive keyboard shortcuts

## 📦 Requirements

- **GCC** or any compatible C compiler
- **Make** build system
- **cURL** library for API requests
- **JSON-C** library for JSON parsing
- **ncurses** library for terminal UI
- **mpv** media player for video playback

## 🚀 Installation

### From Source

1. Clone the repository:

   ```bash
   git clone https://github.com/your-username/anime-cli.git
   cd anime-cli
   ```

2. Build the project:

   ```bash
   make rebuild
   ```

3. Install the program (optional):

   ```bash
   sudo make install
   ```

### From AUR (Arch User Repository)

If you're using an Arch-based distribution:

```bash
# Using your preferred AUR helper, e.g., yay
yay -S anime-cli

# Or with paru
paru -S anime-cli
```

### Manual Dependencies Installation

```bash
# For Arch Linux
sudo pacman -S gcc make curl json-c ncurses mpv

# For Debian/Ubuntu (not tested)
sudo apt install gcc make libcurl4-openssl-dev libjson-c-dev libncurses-dev mpv

# For Fedora (not tested)
sudo dnf install gcc make libcurl-devel json-c-devel ncurses-devel mpv
```

## 📖 Usage

### Basic Commands

Launch anime-cli by running:

```bash
anime-cli
```

### Keyboard Shortcuts

**Search Screen:**

- Type your query and press **Enter**

**Anime Selection:**

- **↑/↓**: Navigate through anime list
- **Enter**: Select anime
- **Type any text**: Filter anime list
- **ESC**: Clear filter
- **q**: Quit to previous screen
- **Ctrl+C**: Exit program

**Episode Selection:**

- **↑/↓**: Navigate through episodes
- **Enter**: Watch selected episode
- **q**: Return to anime search

**Video Playback (MPV):**

- **Space**: Pause/Play
- **→/←**: Seek forward/backward
- **j**: Cycle through subtitle tracks
- **v**: Toggle subtitle visibility
- **q**: Quit playback and return to episode menu

## 📸 Screenshots

| Search Screen | Anime Selection | Episode Selection |
|:---:|:---:|:---:|
| ![Search](/screenshots/Search.png) | ![Selection](/screenshots/Select.png) | ![Episodes](/screenshots/Episodes.png) |

## 🛠️ Development

Run tests:

```bash
make test
```

Code structure:

- main.c - Main application entry point
- ui.c - Terminal user interface
- api.c - API client for Consumet
- utils.c - Utility functions

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a new branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Commit your changes (`git commit -m 'Add some amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

Please ensure your code follows the project's style and includes appropriate tests.

## 📜 License

Distributed under the MIT License. See LICENSE for more information.

```markdown
MIT License

Copyright (c) 2023 Thuan Tran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions...
```

## 🙏 Acknowledgements

- [Consumet API](https://github.com/consumet) - For providing the anime data
- [libcurl](https://curl.se/libcurl/) - For HTTP request handling
- [json-c](https://github.com/json-c/json-c) - For JSON parsing
- [ncurses](https://invisible-island.net/ncurses/) - For terminal UI
- [mpv](https://mpv.io/) - For media playback

---

<div align="center">
  <sub>Built with ❤️ by Thuan Tran and contributors</sub>
</div>
