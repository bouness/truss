# 🏗️ Truss Analysis Tool

A professional structural analysis tool for engineers that calculates truss displacements, reactions, and member forces. Features a modern GUI with CSV import/export capabilities.

![Application Screenshot](src/assets/icon.png)

## Features ✨

- **Advanced Calculation Engine**:
  - 3D truss analysis with constraint handling
  - Member force and stress calculations
  - Support for various material properties

- **Professional Visualization**:
  - Results displayed in formatted ASCII tables
  - Export to text files with proper formatting
  - System theme integration (dark/light mode)

- **Easy Data Handling**:
  - Import/export CSV files
  - Direct data pasting
  - Decimal precision control (1-12 places)

## 💖 Support

Scan to donate via Venmo:

![Venmo QR](https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=https://venmo.com/youness-bougteb)
## 🧰 Tech Stack

- Truss Engine: C
- Truss GUI: Python

## 📦 Installation

### 🪟 Windows
1. Download the latest installer from [Releases](https://github.com/bouness/truss/releases)
2. Run `TrussInstaller.exe`
3. Follow the installation wizard

### 🐧 Linux
1. Download the latest `Truss-linux.tar.gz` from [Releases](https://github.com/bouness/truss/releases)

```bash
tar -xzf Truss-linux.tar.gz
cd truss-*

# to install
sudo ./installer/install.sh

# to remove
sudo ./installer/uninstall.sh
```
## 🛠️ Building from Source

Requirements
* GCC (for C compilation)
* Python 3.10+
* Python packages: PySide6

### Build Process
```bash
# Clone repository
git clone https://github.com/bouness/truss.git
cd truss

# Install dependencies
sudo apt install build-essential python3-pip
pip install PySide6

# Build Linux version
chmod +x build.sh
./build.sh

# Build Windows version (requires MinGW)
./build.bat

# Run in development mode
./run_dev.sh
```

## 📖 Usage Guide
1. Input Data:
    * Load a CSV file or paste data directly
    * Format (inches, kips, ksi):
      ```bash
      # Node_id, x, y, z, fix_x, fix_y, fix_z, load_x, load_y, load_z
      1, 0, 0, 0, 1, 1, 1, 0, 0, 0
      2, 60, 60, 0, 0, 0, 1, 0, -10, 0
      3, 120, 0, 0, 0, 1, 1, 0, 0, 0

      # Member_id, start_node, end_node, E, A
      1, 2, 1, 29000, 0.75
      2, 2, 3, 29000, 0.75
      3, 1, 3, 29000, 0.5
      ```
2. Set Precision:
    * Choose decimal places (6-12) for calculations
3. Run Analysis:
    * Click "Run Analysis" to process your truss
4. View Results:
    * See displacements, reactions, and member forces
    * Save or copy results as needed

## ⚙️ Technical Details
* **Engine**: Custom C solver for high-performance calculations
* **GUI**: PySide6 with system-native interface
* **Packaging**: Nuitka for standalone executables
* **Cross-Platform**: Works on Windows and Linux

## 🤝 Contributing
Contributions are welcome! Please follow these steps:
1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a pull request

## 📄 License MIT
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
