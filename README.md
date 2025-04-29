# Click the Target 🎯  

*A fast-paced target-shooting game built with C++20 and Raylib*

## Features ✨  
- 🎯 Dynamic target mechanics with physics  
- 🎮 Three difficulty levels (Easy, Medium, Hard)  
- 💥 Particle explosion effects on hits  
- 🏆 Persistent binary leaderboard system  
- 🔊 Multi-track music system with sound effects  
- 🖱 Custom crosshair cursor  

## Requirements  
- **CMake 3.29+**  
- **C++20 compatible compiler** (GCC 11+, Clang 12+, MSVC 19.30+)  
- **Raylib 4.0+**  

## Build & Run 🛠️  
```bash
git clone https://github.com/IvanGenev26/clickTheTarget.git
cd clickTheTarget
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./clickTheTargetGame
