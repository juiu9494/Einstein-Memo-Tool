# Einstein Memo Tool V12.0 - Extended Edition
**by juiu9494**

## ❤️ Support
If you find this project useful, you can support its development here:  
[![PayPal](https://img.shields.io/badge/Donate-PayPal-blue?style=for-the-badge&logo=paypal)](https://www.paypal.com/pool/9pfCpQTsk1?sr=wccr)
             
              ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
              █ ▄▄▄▄▄ █  ▄▄▄▄▄ █▀ ▄▄▄▄▄ █▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀█
              █ █   █ █  █   █ █  █   █ █ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █ █▄▄▄█ █  █▄▄▄█ █  █▄▄▄█ █ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █▄▄▄▄▄▄▄█▄▄▄▄▄▄▄█▄▄▄▄▄▄▄█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █  ▄▄▄▄▄ █ ▄▄▄▄▄ █  ▄▄▄▄▄ █▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀█
              █  █   █ █ █   █ █  █   █ █ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █  █▄▄▄█ █ █▄▄▄█ █  █▄▄▄█ █ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █▄▄▄▄▄▄▄█▄▄▄▄▄▄▄█▄▄▄▄▄▄▄█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █  ▄▄▄▄▄ █ ▄▄▄▄▄ █  ▄▄▄▄▄ █▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀█
              █  █   █ █ █   █ █  █   █ █ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █  █▄▄▄█ █ █▄▄▄█ █  █▄▄▄█ █ ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
              █▄▄▄▄▄▄▄█▄▄▄▄▄▄▄█▄▄▄▄▄▄▄█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█


What is this?

        
               ███████╗██╗███╗   ██╗███████╗████████╗███████╗██╗███╗   ██╗
             ██╔════╝██║████╗  ██║██╔════╝╚══██╔══╝██╔════╝██║████╗  ██║
             █████╗  ██║██╔██╗ ██║███████╗   ██║   █████╗  ██║██╔██╗ ██║
             ██╔══╝  ██║██║╚██╗██║╚════██║   ██║   ██╔══╝  ██║██║╚██╗██║
             ███████╗██║██║ ╚████║███████║   ██║   ███████╗██║██║ ╚████║
             ╚══════╝╚═╝╚═╝  ╚═══╝╚══════╝   ╚═╝   ╚══════╝╚═╝╚═╝  ╚═══╝

Einstein Memo Tool is a feature‑rich note manager that lives inside your terminal. It encrypts your data, remembers your searches, supports multiple users, and even checks for corruption. If you love the command line and want to keep your notes secure, this tool was made for you.
Quick Start

    Compile (C++17 required)
    bash

g++ -std=c++17 -O2 -o einstein einstein_memo_tool_v12.cpp
./einstein

    Create an admin profile on first launch.

    Inject a note – press 2 and follow the prompts.

    Search – press 1, type a term (typos are forgiven!).

    Browse – press 3 to see your entries in a table.

    Export – press B for CSV or JSON.

Features

    🔍 Fuzzy search (Levenshtein + Dice) – finds “Einstein” even if you type “Enstein”

    📝 Full CRUD – inject, update, rename, delete entries

    🏷️ Tags, categories & priorities (LOW / MEDIUM / HIGH)

    👥 Multi‑user profiles with hashed passwords and admin rights

    🔐 Encryption – double XOR cipher on all stored files

    🛡️ Integrity checks – per‑entry djb2 checksum

    📊 Statistics dashboard – by category, priority, most accessed

    📁 Import/export – text, CSV, JSON

    💾 Backup & restore – automatic or manual

    🎨 Beautiful TUI – colors, animations, progress bars, pagination

    📜 Audit log – every action timestamped and encrypted

    ⚙️ Fully configurable – toggle colors, animations, debug, page size

Security

All data (memory, history, audit, profiles) is stored encrypted using a two‑pass XOR cipher. Each entry carries a checksum to detect tampering. User passwords are hashed before storage. A protected mode allows read‑only access.
File Formats

Import (plain text):
text

name|information|category|tag1,tag2|PRIORITY

Example:
text

Einstein|Theoretical physicist|Science|physics,history|HIGH

Lines starting with # are skipped.

Export: CSV and JSON with all fields.
Configuration

Press G to adjust:

    Colors on/off

    Animations on/off

    Debug mode

    Auto‑backup

    Animation speed

    Results per page

    Audit logging

All settings are saved in config_ia.txt.

