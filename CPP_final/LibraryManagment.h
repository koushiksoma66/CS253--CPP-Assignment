#ifndef LIBRARY_SYSTEM_H
#define LIBRARY_SYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <chrono>

using namespace std;

class Book;
class Member;
class Account;
class Library;
class Student;
class Professor;
class Librarian;

struct BorrowInfo {
    const Book* book;
    const Member* borrower;
    chrono::system_clock::time_point borrowDate;
    chrono::system_clock::time_point dueDate;
};

class Book {
private:
    int bookID;
    string title;
    string author;
    string publisher;
    int year;
    string ISBN;
    bool available;
    queue<int> reservationQueue;

public:
    Book(int id, const string& title, const string& author, const string& publisher, int year, const string& isbn);
    
    int getBookID() const;
    string getTitle() const;
    string getAuthor() const;
    string getPublisher() const;
    int getYear() const;
    string getISBN() const;
    bool isAvailable() const;
    void setAvailable(bool status);
    
    bool reserve(int userID);
    bool cancelReservation(int userID);
    bool isReserved() const;
    int getNextReservation();
    bool isReservedBy(int userID) const;
};

struct BorrowRecord {
    int bookID;
    chrono::system_clock::time_point borrowDate;
    chrono::system_clock::time_point dueDate;
};

class Account {
private:
    int userID;
    vector<BorrowRecord> currentBorrows;
    vector<BorrowRecord> borrowHistory;
    double totalFine;

public:
    Account(int id);
    
    void addBorrow(int bookID);
    void removeBorrow(int bookID);
    const vector<BorrowRecord>& getCurrentBorrows() const;
    const vector<BorrowRecord>& getBorrowHistory() const;
    double getTotalFine() const;
    void addFine(double amount);
    void payFine(double amount);
    void addToBorrowHistory(const BorrowRecord& record);
};

class Member {
protected:
    int userID;
    string name;
    string password;
    string department;
    string role;

public:
    Member(int id, const string& name, const string& password);
    virtual ~Member() = default;

    int getUserID() const;
    string getName() const;
    string getRole() const;
    string getDepartment() const;
    string getPassword() const { return password; }
    void setDepartment(const string& dept);
    bool verifyPassword(const string& pwd) const { return password == pwd; }

    virtual bool canBorrow() const = 0;
    virtual bool canManageBooks() const = 0;
    virtual bool canManageUsers() const = 0;
    virtual int getMaxBooks() const = 0;
    virtual int getMaxDays() const = 0;
    virtual double getFineRate() const = 0;
};

class Student : public Member {
private:
    static const int MAX_BOOKS = 3;
    static const int MAX_DAYS = 15;
    static constexpr double FINE_RATE = 10.0;

public:
    Student(int id, const string& name, const string& password);
    
    bool canBorrow() const override { return true; }
    bool canManageBooks() const override { return false; }
    bool canManageUsers() const override { return false; }
    int getMaxBooks() const override { return MAX_BOOKS; }
    int getMaxDays() const override { return MAX_DAYS; }
    double getFineRate() const override { return FINE_RATE; }
};

class Professor : public Member {
private:
    static const int MAX_BOOKS = 5;
    static const int MAX_DAYS = 30;
    static constexpr double FINE_RATE = 0.0;
    static const int MAX_OVERDUE_DAYS = 60;

public:
    Professor(int id, const string& name, const string& password);
    
    bool canBorrow() const override { return true; }
    bool canManageBooks() const override { return true; }
    bool canManageUsers() const override { return false; }
    int getMaxBooks() const override { return MAX_BOOKS; }
    int getMaxDays() const override { return MAX_DAYS; }
    double getFineRate() const override { return FINE_RATE; }
};

class Librarian : public Member {
public:
    Librarian(int id, const string& name, const string& password);
    
    bool canBorrow() const override { return false; }
    bool canManageBooks() const override { return true; }
    bool canManageUsers() const override { return true; }
    int getMaxBooks() const override { return 0; }
    int getMaxDays() const override { return 0; }
    double getFineRate() const override { return 0.0; }
};

class Library {
private:
    unordered_map<int, unique_ptr<Book>> books;
    unordered_map<int, unique_ptr<Member>> users;
    unordered_map<int, unique_ptr<Account>> accounts;

    static vector<string> split(const string& str, char delim);
    template<typename Func>
    void readDataFile(const string& filename, Func&& callback);

public:
    Library() = default;
    ~Library();

    bool addBook(unique_ptr<Book> book);
    bool removeBook(int bookID);
    const Book* getBook(int bookID) const;
    vector<const Book*> searchBooks(const string& query) const;

    bool addUser(unique_ptr<Member> user);
    bool removeUser(int userID);
    const Member* getMember(int userID) const;
    bool authenticateUser(int userID, const string& password) const;
    Account* getAccount(int userID) const;

    bool borrowBook(int userID, int bookID);
    bool returnBook(int userID, int bookID);
    bool payFine(int userID, double amount);
    bool reserveBook(int userID, int bookID);
    bool cancelReservation(int userID, int bookID);
    vector<const Book*> getReservedBooks(int userID) const;
    vector<BorrowInfo> getAllBorrowedBooks() const;

    void saveState() const;
    void loadState();
    void loadAccountInfo(int userID);
};

#endif