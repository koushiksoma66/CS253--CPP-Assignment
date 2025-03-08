#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <iomanip>
#include <fstream> // To read and write from files
#include <sstream>
#include <functional>
#include "LibraryManagment.h"

using namespace std;

vector<string> split(const string& str, char delim) {
    string token;
    vector<string> token_array;
    istringstream tokenStream(str); 
    while (getline(tokenStream, token, delim)) {
        token_array.push_back(token);
    }
    return token_array;
}

void readDataFile(const string& filename, function<void(const vector<string>&)> func) { // func function processes each line of the file after splitting it into parts
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "\033[1;31mError: Could not open the file " << filename << "\033[0m" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Empty lines and comments
        
        auto parts = split(line, '|');
        func(parts);
    }
}

void displayMenu();
void displayUserMenu(const Member* member);
void handleAddUser(Library& library);
void handleRemoveUser(Library& library);
void handleCheckUser(const Library& library);
void handleViewFines(const Library& library, int userID);
void handlePayFine(Library& library, int userID);
void clearInputBuffer(); // Function to clear the input buffer, typically used to discard any leftover characters in the input stream. This is useful after reading input to ensure that subsequent input operations work correctly.
void waitForEnter(); // Function to wait for the user until they press enter.
void displayBookDetails(const Book* book);
void handleSearchBooks(const Library& library);
void handleBorrowBook(Library& library, int userID);
void handleReturnBook(Library& library, int userID);
void handleAddBook(Library& library);
void handleRemoveBook(Library& library);
void handleViewAllBooks(const Library& library);
void handleReserveBook(Library& library, int userID);
void handleCancelReservation(Library& library, int userID);
void handleViewReservations(const Library& library, int userID);
void handleViewAllBorrowedBooks(const Library& library);
void initializeLibrary(Library& lib);


void clearInputBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void displayMenu() {
    cout << "\n-------------------------------------\n";
    cout << "    Library Management System\n";
    cout << "-------------------------------------\n";
    cout << "1. Enter 1 to 'Login'\n";
    cout << "2. Enter 2 to 'Exit'\n\n";
    cout << "-------------------------------------\n";
    cout << "Test Accounts:\n";
    cout << "-------------------------------------\n";
    cout << "Students:\n";
    cout << "  ID: 101  Password: student357\n";
    cout << "  ID: 102  Password: student206\n\n";
    cout << "Professors:\n";
    cout << "  ID: 201  Password: prof906\n";
    cout << "  ID: 202  Password: prof488\n\n";
    cout << "Librarian:\n";
    cout << "  ID: 301  Password: lib199\n";
    cout << "-------------------------------------\n\n";
    cout << "Enter your choice (1 or 2): ";
}

void displayUserMenu(const Member* member) {
    cout << "\n-------------------------------------\n";
    cout << "Welcome " << member->getName() << "\n";
    cout << "Role: " << member->getRole() << "\n";
    cout << "-------------------------------------\n\n";
    cout << "1. Search Books\n";
    cout << "2. View All Books\n";
    
    if (member->canBorrow()) {
        cout << "3. Borrow Book\n";
        cout << "4. Return Book\n";
        cout << "5. Reserve Book\n";
        cout << "6. Cancel Reservation\n";
        cout << "7. View Reservations\n";
        cout << "8. View Borrowed Books\n";
        cout << "9. View Fines\n";
        cout << "10. Pay Fine\n";
    }
    
    if (member->canManageBooks()) {
        cout << "11. Add Book\n";
        cout << "12. Remove Book\n";
    }
    
    if (member->canManageUsers()) {
        cout << "13. Add User\n";
        cout << "14. Remove User\n";
        cout << "15. Check User\n";
        cout << "16. View All Borrowed Books\n";
    }
    
    cout << "\n0. Logout\n";
    cout << "\nEnter your choice: ";
}

void handleReturnBook(Library& library, int userID) {
    int bookID;
    cout << "Enter Book ID to return: ";
    cin >> bookID;
    cin.ignore();

    const Book* book = library.getBook(bookID);
    if (!book) {
        cout << "\033[1;31mError: Book not found.\033[0m\n";
        return;
    }

    const Member* member = library.getMember(userID);
    if (!member) {
        cout << "\033[1;31mError: User not found.\033[0m\n";
        return;
    }

    Account* account = library.getAccount(userID);
    if (!account) {
        cout << "\033[1;31mError: Account not found.\033[0m\n";
        return;
    }

    bool hasBorrowed = false;
    for (const auto& borrow : account->getCurrentBorrows()) {
        if (borrow.bookID == bookID) {
            hasBorrowed = true;
            break;
        }
    }

    if (!hasBorrowed) {
        cout << "\033[1;31mError: You have not borrowed this book.\033[0m"<<endl;
        return;
    }

    if (library.returnBook(userID, bookID)) {
        cout << "Book returned successfully!\n";
    }
    else {
        cout << "\033[1;31mError: Failed to return book. Please try again.\033[0m"<<endl;
    }
}

void handleViewFines(const Library& library, int userID) {
    const Account* account = library.getAccount(userID);
    if (account) {
        double fine = account->getTotalFine();
        if (fine > 0) {
            cout << "Total fine: Rs. " << fixed << setprecision(2) << fine << "\n";
        } else {
            cout << "No outstanding fines.\n";
        }
    }
    else{
        cout << "\033[1;31mError: Account not found.\033[0m"<<endl;
    }
}

void handlePayFine(Library& library, int userID) {
    const Account* account = library.getAccount(userID);
    if (!account) {
        cout << "\033[1;31mError: Account not found.\033[0m"<<endl;
        return;
    }
    
    if (account->getTotalFine() == 0) {
        cout << "No fines to pay.\n";
        return;
    }

    double amount;
    cout << "Total fine: Rs. " << fixed << setprecision(2) 
              << account->getTotalFine() << "\n";
    cout << "Enter amount to pay: ";
    cin >> amount;

    if (library.payFine(userID, amount)) {
        cout << "Payment successful!\n";
    } else {
        cout << "Payment failed.\n";
    }
}

void displayBookDetails(const Book* book) {
    if (!book) return;
    cout << "\nBook Details:\n";
    cout << "-------------------------------------\n";
    cout<<"ID  |  Title  |  Author  |  Publisher  |  Year  |  ISBN  |  Status\n";
    cout << "-------------------------------------\n";
    cout << book->getBookID() << "  |  " << book->getTitle() << "  |  " << book->getAuthor() << "  |  " << book->getPublisher() << "  |  " << book->getYear() << "  |  " << book->getISBN() << "  |  " << (book->isAvailable() ? "Available" : "Borrowed") << "\n";
    cout << "-------------------------------------\n";
}

void handleSearchBooks(const Library& library) {
    clearInputBuffer();
    string query;
    cout << "Enter search term (title/author): ";
    getline(cin, query);

    auto results = library.searchBooks(query);
    if (results.empty()) {
        cout << "No books found.\n";
        return;
    }

    cout << "\nFound " << results.size() << " books:\n";
    for (const auto* book : results) {
        displayBookDetails(book);
    }
}

void handleBorrowBook(Library& library, int userID) {
    int bookID;
    cout << "Enter Book ID to borrow: ";
    cin >> bookID;
    cin.ignore();

    const Book* book = library.getBook(bookID);
    if (!book) {
        cout << "\033[1;31mError: Book not found.\033[0m\n";
        return;
    }

    const Member* member = library.getMember(userID);
    if (!member) {
        cout << "\033[1;31mError: User not found.\033[0m\n";
        return;
    }

    Account* account = library.getAccount(userID);
    if (!account) {
        cout << "\033[1;31mError: Account not found.\033[0m\n";
        return;
    }

    if (!member->canBorrow()) {
        cout << "\033[1;31mError: Your role (" << member->getRole() << ") is not allowed to borrow books.\033[0m\n";
        return;
    }

    if (!book->isAvailable()) {
        cout << "\033[1;31mError: This book is currently borrowed.\033[0m\n";
        cout << "You can reserve it for when it becomes available.\n";
        return;
    }

    if (account->getCurrentBorrows().size() >= member->getMaxBooks()) {
        cout << "\033[1;31mError: You have reached your borrowing limit of " << member->getMaxBooks() << " books.\033[0m\n";
        return;
    }

    for (const auto& borrow : account->getCurrentBorrows()) {
        if (borrow.bookID == bookID) {
            cout << "Error: You have already borrowed this book.\n";
            return;
        }
    }

    if (account->getTotalFine() > 0) {
        cout << "Error: You have outstanding fines of Rs. " << fixed << setprecision(2) << account->getTotalFine() << ". Please pay your fines before borrowing.\n";
        return;
    }

    if (library.borrowBook(userID, bookID)) {
        cout << "Book borrowed successfully!\n";
        const auto& borrows = account->getCurrentBorrows();
        time_t dueTime = chrono::system_clock::to_time_t(borrows.back().dueDate);
        cout << "Due date: " << ctime(&dueTime);
    } else {
        cout << "\033[1;31mError: Failed to borrow book. Please try again.\033[0m\n";
    }
}


void handleViewBorrowedBooks(const Library& library, int userID) {
    const Account* account = library.getAccount(userID);
    if (!account || account->getCurrentBorrows().empty()) {
        cout << "No borrowed books.\n";
        return;
    }

    cout << "\nBorrowed Books:\n";
    for (const auto& record : account->getCurrentBorrows()) {
        const Book* book = library.getBook(record.bookID);
        if (book) {
            displayBookDetails(book);
        }
    }
}

void handleAddBook(Library& library) {
    int bookID;
    string title, author, publisher, isbn;
    int year;

    cout << "\nAdd New Book\n";
    cout << "Enter Book ID: ";
    cin >> bookID;
    clearInputBuffer();

    cout << "Enter Title: "; getline(cin, title);

    cout << "Enter Author: "; getline(cin, author);

    cout << "Enter Publisher: "; getline(cin, publisher);

    cout << "Enter Year: "; cin >> year;
    clearInputBuffer();

    cout << "Enter ISBN: "; getline(cin, isbn);

    auto book = make_unique<Book>(bookID, title, author, publisher, year, isbn);
    if (library.addBook(move(book))) {
        cout << "Book added successfully!\n";
    } else {
        cout << "Failed to add book.\n";
    }
}

void handleRemoveBook(Library& library) {
    int bookID;
    cout << "Enter Book ID to remove: ";
    cin >> bookID;

    if (library.removeBook(bookID)) {
        cout << "Book removed successfully!\n";
    } else {
        cout << "Failed to remove book.\n";
    }
}

void handleAddUser(Library& library) {
    int userID;
    string name, password, department;
    char userType;

    cout << "\nAdd New User\n";
    cout << "User Type (Student/Professor/Librarian): ";
    cin >> userType;
    cin.ignore();

    cout << "Enter ID: ";
    cin >> userID;
    cin.ignore();

    cout << "Enter Name: ";
    getline(cin, name);

    cout << "Enter Password: ";
    getline(cin, password);

    cout << "Enter Department: ";
    getline(cin, department);

    unique_ptr<Member> user;
    switch (toupper(userType)) {
        case 'S':
            user = make_unique<Student>(userID, name, password);
            break;
        case 'P':
            user = make_unique<Professor>(userID, name, password);
            break;
        case 'L':
            user = make_unique<Librarian>(userID, name, password);
            break;
        default:
            cout << "Invalid user type!\n";
            return;
    }

    user->setDepartment(department);
    if (library.addUser(move(user))) {
        cout << "User added successfully!\n";
    } else {
        cout << "Failed to add user.\n";
    }
}

void handleRemoveUser(Library& library) {
    int userID;
    cout << "Enter User ID to remove: ";
    cin >> userID;

    if (library.removeUser(userID)) {
        cout << "User removed successfully!\n";
    } else {
        cout << "Failed to remove user.\n";
    }
}

void handleCheckUser(const Library& library) {
    int userID;
    cout << "Enter User ID to check: ";
    cin >> userID;

    const Member* member = library.getMember(userID);
    if (member) {
        cout << "\nUser Details:\n";
        cout << "ID: " << member->getUserID() << "\n";
        cout << "Name: " << member->getName() << "\n";
        cout << "Role: " << member->getRole() << "\n";
        cout << "Department: " << member->getDepartment() << "\n";
    } else {
        cout << "User not found.\n";
    }
}

void handleViewAllBooks(const Library& library) {
    cout << "\n--- All Books in Library ---\n";
    
    auto books = library.searchBooks("");
    if (books.empty()) {
        cout << "No books in the library.\n";
        return;
    }

    cout << "\nTotal Books: " << books.size() << "\n";
    cout << "--------------------\n";
    
    for (const auto* book : books) {
        displayBookDetails(book);
        cout << "--------------------\n";
    }
}

void handleReserveBook(Library& library, int userID) {
    int bookID;
    cout << "Enter Book ID to reserve: ";
    cin >> bookID;
    cin.ignore();

    const Book* book = library.getBook(bookID);
    if (!book) {
        cout << "\033[1;31mError: Book not found.\033[0m\n";
        return;
    }

    if (book->isReservedBy(userID)) {
        cout << "\033[1;31mError: You already have a reservation for this book.\033[0m\n";
        return;
    }

    if (book->isAvailable()) {
        cout << "\033[1;31mError: This book is currently available. You can borrow it directly.\033[0m\n";
        return;
    }

    if (library.reserveBook(userID, bookID)) {
        cout << "Book reserved successfully!\n";
    } else {
        cout << "\033[1;31mError: Failed to reserve book. Please try again.\033[0m\n";
    }
}

void handleCancelReservation(Library& library, int userID) {
    int bookID;
    cout << "Enter Book ID to cancel reservation: ";
    cin >> bookID;
    cin.ignore();

    const Book* book = library.getBook(bookID);
    if (!book) {
        cout << "\033[1;31mError: Book not found.\033[0m\n";
        return;
    }

    if (book->isReservedBy(userID)) {
        if (library.cancelReservation(userID, bookID)) {
            cout << "Reservation cancelled successfully!\n";
        } else {
            cout << "\033[1;31mError: Failed to cancel reservation. Please try again.\033[0m\n";
        }
    } else {
        cout << "\033[1;31mError: You don't have a reservation for this book.\033[0m\n";
    }
}

void handleViewReservations(const Library& library, int userID) {
    auto books = library.getReservedBooks(userID);
    if (books.empty()) {
        cout << "You have no book reservations.\n";
        return;
    }

    cout << "\nYour Reserved Books:\n";
    cout << "--------------------\n";
    for (const auto* book : books) {
        displayBookDetails(book);
        cout << "--------------------\n";
    }
}

void handleViewAllBorrowedBooks(const Library& library) {
    auto borrowedBooks = library.getAllBorrowedBooks();
    if (borrowedBooks.empty()) {
        cout << "No books are currently borrowed.\n";
        return;
    }

    cout << "\n--- Currently Borrowed Books ---\n\n";
    for (const auto& info : borrowedBooks) {
        cout << "Book Details:\n";
        cout << "-------------\n";
        displayBookDetails(info.book);
        cout << "\nBorrower Details:\n";
        cout << "----------------\n";
        cout << "ID: " << info.borrower->getUserID() << "\n";
        cout << "Name: " << info.borrower->getName() << "\n";
        cout << "Role: " << info.borrower->getRole() << "\n";
        cout << "Department: " << info.borrower->getDepartment() << "\n";
        
        // Converting time_points to a readable format
        auto borrowTime = chrono::system_clock::to_time_t(info.borrowDate);
        auto dueTime = chrono::system_clock::to_time_t(info.dueDate);
        
        cout << "\nBorrow Date: " << ctime(&borrowTime);
        cout << "Due Date: " << ctime(&dueTime);
        cout << "============================\n\n";
    }
}

void initializeLibrary(Library& lib) {
    readDataFile("data/books.txt", [&lib](const auto& parts) {
        if (parts.size() == 7) {  // Changed from 6 to 7 to match the save format
            int id = stoi(parts[0]);
            int year = stoi(parts[4]);
            bool available = parts[6] == "1";
            auto book = make_unique<Book>(
                id, parts[1], parts[2], parts[3], year, parts[5]
            );
            book->setAvailable(available);
            lib.addBook(move(book));
        }
    });

    readDataFile("data/users/students.txt", [&lib](const auto& parts) {
        if (parts.size() == 4) {
            int id = stoi(parts[0]);
            auto student = make_unique<Student>(id, parts[1], parts[2]);
            student->setDepartment(parts[3]);
            lib.addUser(move(student));
            lib.loadAccountInfo(id);
        }
    });

    readDataFile("data/users/professors.txt", [&lib](const auto& parts) {
        if (parts.size() == 4) {
            int id = std::stoi(parts[0]);
            auto faculty = std::make_unique<Professor>(id, parts[1], parts[2]);
            faculty->setDepartment(parts[3]);
            lib.addUser(std::move(faculty));
            lib.loadAccountInfo(id);
        }
    });

    readDataFile("data/users/librarians.txt", [&lib](const auto& parts) {
        if (parts.size() == 4) {
            int id = std::stoi(parts[0]);
            auto librarian = std::make_unique<Librarian>(id, parts[1], parts[2]);
            librarian->setDepartment(parts[3]);
            lib.addUser(std::move(librarian));
            lib.loadAccountInfo(id);
        }
    });
}

int main() {
    Library library;
    initializeLibrary(library);

    while (true) {
        displayMenu();
        int choice;
        cin >> choice;

        if (choice == 2) {
            library.saveState();
            cout << "Thank you for using the Library Management System!\n";
            waitForEnter();
            break;
        }

        if (choice == 1) {
            int userID;
            string password;
            cout << "Enter User ID: ";
            cin >> userID;
            cout << "Enter Password: ";
            cin >> password;

            if (library.authenticateUser(userID, password)) {
                const Member* member = library.getMember(userID);
                if (member) {
                    while (true) {
                        displayUserMenu(member);
                        int userChoice;
                        cin >> userChoice;

                        if (userChoice == 0) {
                            library.saveState();
                            cout << "Logging out...\n";
                            waitForEnter();
                            break;
                        }

                        switch (userChoice) {
                            case 1: 
                                handleSearchBooks(library); 
                                waitForEnter();
                                break;
                            case 2:
                                handleViewAllBooks(library);
                                waitForEnter();
                                break;
                            case 3:
                                if (member->canBorrow()) {
                                    handleBorrowBook(library, userID);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 4:
                                if (member->canBorrow()) {
                                    handleReturnBook(library, userID);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 5:
                                if (member->canBorrow()) {
                                    handleReserveBook(library, userID);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 6:
                                if (member->canBorrow()) {
                                    handleCancelReservation(library, userID);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 7:
                                if (member->canBorrow()) {
                                    handleViewReservations(library, userID);
                                    waitForEnter();
                                }
                                break;
                            case 8:
                                if (member->canBorrow()) {
                                    handleViewBorrowedBooks(library, userID);
                                    waitForEnter();
                                }
                                break;
                            case 9:
                                if (member->canBorrow()) {
                                    handleViewFines(library, userID);
                                    waitForEnter();
                                }
                                break;
                            case 10:
                                if (member->canBorrow()) {
                                    handlePayFine(library, userID);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 11:
                                if (member->canManageBooks()) {
                                    handleAddBook(library);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 12:
                                if (member->canManageBooks()) {
                                    handleRemoveBook(library);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 13:
                                if (member->canManageUsers()) {
                                    handleAddUser(library);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 14:
                                if (member->canManageUsers()) {
                                    handleRemoveUser(library);
                                    library.saveState();
                                    waitForEnter();
                                }
                                break;
                            case 15:
                                if (member->canManageUsers()) {
                                    handleCheckUser(library);
                                    waitForEnter();
                                }
                                break;
                            case 16:
                                if (member->canManageUsers()) {
                                    handleViewAllBorrowedBooks(library);
                                    waitForEnter();
                                }
                                break;
                            default: 
                                cout << "Invalid choice!\n";
                                waitForEnter();
                                break;
                        }
                    }
                }
            } else {
                cout << "Invalid credentials!\n";
                waitForEnter();
            }
        } else {
            cout << "Invalid choice!\n";
            waitForEnter();
        }
    }

    return 0;
} 