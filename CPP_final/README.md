# Library Management System

A C++ OOPs based Library Management System that allows for user and management, and for borrowing and reserving operations.

## Features

- Multiple user roles (Student, Faculty, Librarian) with different privileges
- Book management (add, remove, search)
- User management (add, remove, check)
- Borrowing system with fine calculation
- Reservation system for unavailable books
- File-based data storage
- Automatic fine calculation for overdue books
- Comprehensive error handling

## Project Structure 

```
Library Management System/
├── main.cpp                 # Main program entry point
├── LibrarySystem.h          # Main header file with class declarations
├── LibrarySystem.cpp       # Implementation of library system classes
└── data/                  # Data storage directory
    └── users/          # Users data
      └──  books.txt          # Book information
      ├── students.txt       # Student user data
      ├── faculty.txt        # Faculty user data
      ├── librarians.txt     # Librarian user data
   └── accounts/          # User account data
        └── *.txt          # Individual account files
```

## How to Compile and Run

### Prerequisites
- G++ compiler
- C++11 or higher

### Compilation
Open terminal in the project root directory and run:
```bash
g++ *.cpp -o main
```

### Running the Program
After compilation:
  ```bash
  ./main
  ```

## Test Accounts

### Students (can borrow up to 3 books)
- ID: 101, Password: pass123
- ID: 102, Password: pass456

### Professor (can borrow up to 5 books + manage books)
- ID: 201, Password: faculty123
- ID: 202, Password: faculty456

### Librarian (full access)
- ID: 301, Password: admin123

## User options

### Students
- Search books
- View all books
- View borrowed books
- Borrow books (max 3)
- Return books
- Reserve unavailable books
- Cancel reservations
- View reservations
- View fines
- Pay fines

### Professor
- Borrow up to 5 books
- Add new books
- Remove books
Added to all student's options

### Librarian
- Search books
- View all books
- Add new users
- Remove users
- Check user details
- View all borrowed books

## File Formats

### books.txt
```
BookID|Title|Author|Publisher|Year|ISBN|Available
```

### students.txt, professors.txt, librarians.txt
```
UserID|Name|Password|Department
```

### accounts/*.txt
```
BORROW|BookID|BorrowDate|DueDate
HISTORY|BookID|BorrowDate|DueDate
FINE|Amount
```

## Error Handling

The system handles various errors including:
- Returning unborrowed books
- Borrowing limit exceeded
- Outstanding fines
- Book not available
- Invalid input
- Multiple reservations for same book
- Invalid credentials
- Invalid user roles

## Notes

- The system uses file-based storage
- Fines are calculated based on user type and overdue duration
- Books can be searched by title or author
- Each user type has different borrowing limits and privileges
- Reservations are automatically processed when books are returned
- Account data is stored in separate files for each user