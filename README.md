# 🚇 Metro Transit System

A **C++ console-based Metro Transit Management System** developed as a Project-Based Learning (PBL) project. The application demonstrates object-oriented programming and data structures to organize passenger information, fare calculation, ticket management, stations, and waiting queues.

## 📌 Project Overview

The Metro Transit System is designed to simulate core operations of a metro transit management application. It uses a modular C++ architecture in which different classes manage specific responsibilities.

The project focuses on applying concepts learned in C++ and Data Structures, including inheritance, polymorphism, linked lists, queues, and stacks.

## ✨ Key Features

- **Passenger Management:** Maintains passenger IDs and names using a common passenger class.
- **Fare Calculation:** Calculates standard fares for regular passengers and applies a 50% discount for student passengers.
- **Station Management:** Organizes station-related information.
- **Train Management:** Includes a train management module.
- **Ticket Management:** Organizes ticket-related operations.
- **Student Registry:** Provides a module for student-related records.
- **Waiting Queue:** Manages passengers using queue-based operations.
- **Action Stack:** Uses stack-based logic to manage recent actions.
- **Modular Architecture:** Separates class declarations and implementations into header (`.h`) and source (`.cpp`) files.

*Note: The available operations depend on the implementation in the source code.*

## 🛠️ Technologies Used

- **Programming Language:** C++
- **Core Concepts:** Object-Oriented Programming (OOP)
- **Data Structures:** Linked Lists, Queues, and Stacks
- **OOP Principles:** Classes, Objects, Encapsulation, Inheritance, Function Overriding, and Runtime Polymorphism
- **Development Tools:** Any compatible C++ compiler, such as GCC or MinGW

## 🧠 Concepts Implemented

| Concept | Application |
|---|---|
| Classes and Objects | Represents passengers and other metro entities |
| Encapsulation | Groups data and related functions |
| Inheritance | Reuses common passenger properties |
| Polymorphism | Supports passenger-specific fare calculations |
| Constructors and Destructors | Manage object initialization and destruction |
| Linked Lists | Organizes dynamically linked records |
| Queue (FIFO) | Processes passengers in arrival order |
| Stack (LIFO) | Manages actions in last-in, first-out order |
| Header and Source Files | Separates declarations from implementations |

## 👥 Passenger Fare System

The passenger module contains a base class, `Passenger`, and two derived classes: `RegularPassenger` and `StudentPassenger`.

| Passenger Type | Fare Rule |
|---|---|
| Regular Passenger | Distance × 10 |
| Student Passenger | (Distance × 10) ÷ 2 |

For example, for a distance of 5 units:

- Regular passenger fare: 50
- Student passenger fare: 25

The `calculateFare()` function is virtual in the base class and overridden in the derived classes, demonstrating runtime polymorphism.

## 📂 Project Structure

The project is organized into separate modules. The following illustrates the known class and module organization; the exact repository file list may differ.

```text
Metro-Transit-System-PBL/
│
├── main.cpp
├── Passenger.h
├── Passenger.cpp
├── WaitingQueue.h
├── WaitingQueue.cpp
│
├── StationList.h / StationList.cpp
├── StudentRegistry.h / StudentRegistry.cpp
├── Train.h / Train.cpp
├── TicketList.h / TicketList.cpp
├── ActionStack.h / ActionStack.cpp
│
├── Metro.h / Metro.cpp
└── README.md
```

### Module Responsibilities

- **Passenger:** Stores passenger details and calculates fares.
- **WaitingQueue:** Manages waiting passengers.
- **StationList:** Handles station-related data.
- **StudentRegistry:** Manages student records.
- **Train:** Handles train-related functionality.
- **TicketList:** Manages ticket-related data.
- **ActionStack:** Maintains action history using stack logic.
- **Metro:** Coordinates the system's modules and application flow.
- **main.cpp:** Provides the program entry point.

## ⚙️ Getting Started

### Prerequisites

Install a C++ compiler such as GCC or MinGW and ensure it is available in your terminal.

### 1. Clone the repository

```bash
git clone https://github.com/Hardikrawat05/Metro-Transit-System-PBL.git
```

### 2. Open the project directory

```bash
cd Metro-Transit-System-PBL
```

### 3. Compile the project

If all required `.cpp` files are in the same directory, compile them together:

```bash
g++ -std=c++11 *.cpp -o metro
```

If your project contains multiple directories, compile the required source files using their correct paths instead.

### 4. Run the application

**Linux / macOS / compatible terminals**

```bash
./metro
```

**Windows**

```bash
metro.exe
```

These commands assume that the source files are compatible with the specified compiler and that all required definitions are included.

## 🎯 Project Objectives

- Apply C++ object-oriented programming concepts to a practical problem.
- Implement and understand fundamental data structures.
- Organize a larger program into reusable, maintainable modules.
- Practice collaborative development using Git and GitHub.
- Understand how different modules interact in a metro transit management system.

## 🚀 Future Improvements

Potential enhancements include:

- Persistent storage of passenger and ticket records.
- More advanced route and fare management.
- Additional passenger categories and configurable discount policies.
- Improved input validation and error handling.
- Expanded testing for queue, stack, and ticket operations.

These are potential enhancements, not claims about existing functionality.

## 👨‍💻 Contributors

Developed collaboratively by a team of four as part of a college Project-Based Learning (PBL) initiative.

Contributions cover different modules of the system, with each member responsible for assigned components and integration.

## 📚 Learning Outcomes

Through this project, we explore how object-oriented programming and data structures can work together in a practical application. The project also provides experience with modular code organization, team collaboration, debugging, and version control.

## 🔗 Repository

[Metro Transit System — GitHub](https://github.com/Hardikrawat05/Metro-Transit-System-PBL)

---

**Built with C++ | Object-Oriented Programming | Data Structures | Project-Based Learning**
