# PDFManager

A simple C++ application that manages PDF files by automatically registering new PDFs and removing expired ones.

## Overview

PDFManager helps organize PDF files stored in a local folder. It maintains metadata for registered PDFs and automatically removes files that have exceeded their validity period.

## Features

- **Automatic Registration:** Detects and registers new PDF files placed in the `PDFs` folder when the application starts.
- **Metadata Management:** Stores each PDF's filename, registration date, and expiry date in `metadata.json`.
- **Expiry Tracking:** Sets the expiry date to 15 days after registration.
- **Duplicate Prevention:** Prevents already registered PDFs from being registered again.
- **Automatic Cleanup:** Removes expired PDFs and their corresponding metadata records during application startup.
- **Manual Cleanup:** Allows users to trigger expired-file cleanup through the application menu.
- **Empty Folder Handling:** Handles an empty `PDFs` folder without failing.

## Project Structure

```text
PDFManager/
├── include/
├── PDFs/
├── main.cpp
├── metadata.json
├── .gitignore
└── README.md
```

## Requirements

- A C++ compiler supporting C++11 or later
- A terminal or command-line environment

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/Misbah374/AutoPDF-Cleaner.git
cd AutoPDF-Cleaner
```

### 2. Compile the application

If your source code is contained in `main.cpp`, compile it using:

```bash
g++ main.cpp -o PDFManager
```

On Windows, run the generated executable according to your compiler environment.

> If the project uses additional `.cpp` files, include them in the compilation command.

### 3. Run the application

Place the PDF files you want to manage inside the `PDFs` folder, then run the compiled application.

The application will register new PDFs and clean up expired files according to its configured expiry rules.

## Metadata Format

The application stores PDF information in `metadata.json`, including:

- `filename` — name of the PDF file
- `registeredOn` — date the PDF was registered
- `expiresOn` — date the PDF expires

## Expiry Policy

Each PDF expires 15 days after its registration date. Expired files are removed during startup or through the manual cleanup option.

## Important Notes

- Keep personal and confidential PDFs out of the public GitHub repository.
- The `PDFs` folder is intended for local PDF storage.
- Compiled executables and personal PDF files are excluded using `.gitignore`.

## Future Improvements

- Download PDFs directly from URLs.
- Improve error handling and metadata validation.
- Add more flexible expiry settings.

## Author

**Misbah**
