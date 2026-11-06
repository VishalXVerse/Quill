# Quill C++ OOP Project

## Build

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic Core.cpp Models.cpp Storage.cpp main.cpp -o Quill.exe
```

## Run

```powershell
.\Quill.exe
```

Choose menu option **11** to run the Exception Handling demonstration. It shows the custom Quill exception hierarchy, correct derived-to-base catch order, stack unwinding, RAII file cleanup, `catch(...)` with rethrow, built-in exceptions, and `std::exception` catching.

## C++17 note

C++ has no Java `finally` or `throws` keyword. This project uses RAII (`FileGuard`), cleanup after try/catch, `catch(...)` plus `throw;`, documentation comments, and `noexcept(false)` instead. The former `throw(Type)` dynamic exception specification is removed in C++17 and is deliberately not compiled.
