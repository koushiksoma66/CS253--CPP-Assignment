#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "LibraryManagment.h"

using namespace std;


Book::Book(int id, const string& title, const string& author, 
           const string& publisher, int year, const string& isbn)
    : bookID(id), title(title), author(author), publisher(publisher), 
      year(year), ISBN(isbn), available(true) {}

int Book::getBookID() const { return bookID; }
string Book::getTitle() const { return title; }
string Book::getAuthor() const { return author; }
string Book::getPublisher() const { return publisher; }
int Book::getYear() const { return year; }
string Book::getISBN() const { return ISBN; }
bool Book::isAvailable() const { return available; }
void Book::setAvailable(bool status) { available = status; }

bool Book::reserve(int userID) {
    if (isReservedBy(userID)) {
        return false;
    }
    
    if (!available) {
        reservationQueue.push(userID);
        return true;
    }
    return false;
}

bool Book::cancelReservation(int userID) {
    if (reservationQueue.empty() || !isReservedBy(userID)) return false;
    
    queue<int> tempQueue;
    bool found = false;
    
    while (!reservationQueue.empty()) {
        int currentID = reservationQueue.front();
        reservationQueue.pop();
        
        if (currentID != userID) {
            tempQueue.push(currentID);
        } else {
            found = true;
        }
    }
    
    reservationQueue = tempQueue;
    return found;
}

bool Book::isReserved() const {
    return !reservationQueue.empty();
}

int Book::getNextReservation() {
    if (reservationQueue.empty()) return -1;
    int nextUser = reservationQueue.front();
    reservationQueue.pop();
    return nextUser;
}

bool Book::isReservedBy(int userID) const {
    queue<int> tempQueue = reservationQueue;
    while (!tempQueue.empty()) {
        if (tempQueue.front() == userID) return true;
        tempQueue.pop();
    }
    return false;
}

Account::Account(int id) : userID(id), totalFine(0.0) {}

void Account::addBorrow(int bookID) {
    BorrowRecord record{bookID, 
                       chrono::system_clock::now(),
                       chrono::system_clock::now() + chrono::hours(24*30)};
    currentBorrows.push_back(record);
}

void Account::removeBorrow(int bookID) {
    auto it = find_if(currentBorrows.begin(), currentBorrows.end(),
        [bookID](const BorrowRecord& record) { return record.bookID == bookID; });
    
    if (it != currentBorrows.end()) {
        borrowHistory.push_back(*it);
        currentBorrows.erase(it);
    }
}

const vector<BorrowRecord>& Account::getCurrentBorrows() const { return currentBorrows; }
const vector<BorrowRecord>& Account::getBorrowHistory() const { return borrowHistory; }
double Account::getTotalFine() const { return totalFine; }
void Account::addFine(double amount) { totalFine += amount; }
void Account::payFine(double amount) { totalFine = max(0.0, totalFine - amount); }
void Account::addToBorrowHistory(const BorrowRecord& record) { borrowHistory.push_back(record); }

Member::Member(int id, const string& name, const string& password)
    : userID(id), name(name), password(password) {}

int Member::getUserID() const { return userID; }
string Member::getName() const { return name; }
string Member::getRole() const { return role; }
string Member::getDepartment() const { return department; }
void Member::setDepartment(const string& dept) { department = dept; }

Student::Student(int id, const string& name, const string& password)
    : Member(id, name, password) {
    role = "Student";
}

Professor::Professor(int id, const string& name, const string& password)
    : Member(id, name, password) {
    role = "Professor";
}

Librarian::Librarian(int id, const string& name, const string& password)
    : Member(id, name, password) {
    role = "Librarian";
}

Library::~Library() = default;

template<typename Func>
void Library::readDataFile(const string& filename, Func&& callback) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            auto parts = split(line, '|');
            callback(parts);
        }
        file.close();
    }
}

bool Library::addBook(unique_ptr<Book> book) {
    int bookID = book->getBookID();
    if (books.find(bookID) != books.end()) return false;
    books[bookID] = move(book);
    return true;
}

bool Library::removeBook(int bookID) {
    return books.erase(bookID) > 0;
}

bool Library::addUser(unique_ptr<Member> user) {
    int userID = user->getUserID();
    if (users.find(userID) != users.end()) return false;
    accounts[userID] = make_unique<Account>(userID);
    users[userID] = move(user);
    return true;
}

bool Library::removeUser(int userID) {
    accounts.erase(userID);
    return users.erase(userID) > 0;
}

bool Library::borrowBook(int userID, int bookID) {
    auto userIt = users.find(userID);
    auto bookIt = books.find(bookID);
    
    if (userIt == users.end() || bookIt == books.end()) return false;
    
    if (!userIt->second->canBorrow()) return false;
    
    if (!bookIt->second->isAvailable()) return false;
    
    auto account = accounts[userID].get();
    
    if (account->getCurrentBorrows().size() >= userIt->second->getMaxBooks()) return false;
    
    for (const auto& borrow : account->getCurrentBorrows()) {
        if (borrow.bookID == bookID) return false;
    }
    
    if (account->getTotalFine() > 0) return false;
    
    bookIt->second->setAvailable(false);
    account->addBorrow(bookID);
    saveState(); 
    return true;
}

bool Library::returnBook(int userID, int bookID) {
    auto userIt = users.find(userID);
    auto bookIt = books.find(bookID);
    
    if (userIt == users.end() || bookIt == books.end()) return false;
    
    auto account = accounts[userID].get();
    if (!account) return false;
    
    bool hasBorrowed = false;
    for (const auto& borrow : account->getCurrentBorrows()) {
        if (borrow.bookID == bookID) {
            hasBorrowed = true;
            break;
        }
    }
    if (!hasBorrowed) return false;
    
    auto now = chrono::system_clock::now();
    for (const auto& borrow : account->getCurrentBorrows()) {
        if (borrow.bookID == bookID && now > borrow.dueDate) {
            auto overdueHours = chrono::duration_cast<chrono::hours>(now - borrow.dueDate).count();
            double fine = overdueHours * userIt->second->getFineRate();
            account->addFine(fine);
            break;
        }
    }
    
    account->removeBorrow(bookID);
    
    if (bookIt->second->isReserved()) {
        int nextUserID = bookIt->second->getNextReservation();
        if (nextUserID != -1) {
            borrowBook(nextUserID, bookID);
        } else {
            bookIt->second->setAvailable(true);
            saveState(); 
        }
    } else {
        bookIt->second->setAvailable(true);
        saveState(); 
    }
    
    return true;
}

bool Library::authenticateUser(int userID, const string& password) const {
    auto it = users.find(userID);
    return (it != users.end() && it->second->verifyPassword(password));
}

bool Library::payFine(int userID, double amount) {
    auto accountIt = accounts.find(userID);
    if (accountIt == accounts.end()) return false;
    accountIt->second->payFine(amount);
    return true;
}

const Book* Library::getBook(int bookID) const {
    auto it = books.find(bookID);
    return it != books.end() ? it->second.get() : nullptr;
}

const Member* Library::getMember(int userID) const {
    auto it = users.find(userID);
    return it != users.end() ? it->second.get() : nullptr;
}

Account* Library::getAccount(int userID) const {
    auto it = accounts.find(userID);
    return it != accounts.end() ? it->second.get() : nullptr;
}

vector<const Book*> Library::searchBooks(const string& query) const {
    vector<const Book*> results;
    string lowerQuery = query;
    transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& pair : books) {
        string title = pair.second->getTitle();
        transform(title.begin(), title.end(), title.begin(), ::tolower);
        if (title.find(lowerQuery) != string::npos) {
            results.push_back(pair.second.get());
        }
    }
    return results;
}

bool Library::reserveBook(int userID, int bookID) {
    auto bookIt = books.find(bookID);
    if (bookIt == books.end()) return false;
    bool success = bookIt->second->reserve(userID);
    if (success) {
        saveState(); 
    }
    return success;
}

bool Library::cancelReservation(int userID, int bookID) {
    auto bookIt = books.find(bookID);
    if (bookIt == books.end()) return false;
    bool success = bookIt->second->cancelReservation(userID);
    if (success) {
        saveState(); 
    }
    return success;
}

vector<const Book*> Library::getReservedBooks(int userID) const {
    vector<const Book*> reservedBooks;
    for (const auto& pair : books) {
        if (pair.second->isReservedBy(userID)) {
            reservedBooks.push_back(pair.second.get());
        }
    }
    return reservedBooks;
}

void Library::saveState() const {
    system("mkdir data 2>nul");
    system("mkdir data\\accounts 2>nul");

    ofstream bookFile("data/books.txt");
    if (!bookFile.is_open()) {
        cerr << "Error: Could not open books.txt for writing" << endl;
        return;
    }
    
    for (const auto& pair : books) {
        const auto& book = pair.second;
        bookFile << pair.first << "|" << book->getTitle() << "|" << book->getAuthor() 
                 << "|" << book->getPublisher() << "|" << book->getYear() 
                 << "|" << book->getISBN() << "|" << book->isAvailable() << "\n";
    }
    bookFile.close();

    ofstream studentFile("data/users/students.txt");
    ofstream professorFile("data/users/professors.txt");
    ofstream librarianFile("data/users/librarians.txt");

    if (!studentFile.is_open() || !professorFile.is_open() || !librarianFile.is_open()) {
        cerr << "Error: Could not open user files for writing" << endl;
        return;
    }

    for (const auto& pair : users) {
        const auto& user = pair.second;
        string userLine = to_string(pair.first) + "|" + user->getName() + "|" + 
                         user->getPassword() + "|" + user->getDepartment() + "\n";
        
        if (user->getRole() == "Student") {
            studentFile << userLine;
        } else if (user->getRole() == "Professor") {
            professorFile << userLine;
        } else if (user->getRole() == "Librarian") {
            librarianFile << userLine;
        }
    }

    studentFile.close();
    professorFile.close();
    librarianFile.close();

    for (const auto& pair : accounts) {
        const auto& account = pair.second;
        string accountPath = "data/accounts/" + to_string(pair.first) + ".txt";
        ofstream accountFile(accountPath);
        
        if (!accountFile.is_open()) {
            cerr << "Error: Could not open account file for writing: " << accountPath << endl;
            continue;
        }
        
        for (const auto& record : account->getCurrentBorrows()) {
            accountFile << "BORROW|" << record.bookID << "|"
                       << chrono::system_clock::to_time_t(record.borrowDate) << "|"
                       << chrono::system_clock::to_time_t(record.dueDate) << "\n";
        }
        
        for (const auto& record : account->getBorrowHistory()) {
            accountFile << "HISTORY|" << record.bookID << "|"
                       << chrono::system_clock::to_time_t(record.borrowDate) << "|"
                       << chrono::system_clock::to_time_t(record.dueDate) << "\n";
        }
        
        accountFile << "FINE|" << account->getTotalFine() << "\n";
        
        accountFile.close();
    }
}

void Library::loadState() {
    cout << "Loading state..." << endl;
    
    books.clear();
    users.clear();
    accounts.clear();

    cout << "Loading books..." << endl;
    readDataFile("data/books.txt", [this](const auto& parts) {
        if (parts.size() == 7) {
            int id = stoi(parts[0]);
            int year = stoi(parts[4]);
            bool available = parts[6] == "1";
            auto book = make_unique<Book>(id, parts[1], parts[2], parts[3], year, parts[5]);
            book->setAvailable(available);
            addBook(move(book));
            cout << "Loaded book: " << parts[1] << " (ID: " << id << ")" << endl;
        }
    });
    cout << "Total books loaded: " << books.size() << endl;

    cout << "Loading students..." << endl;
    readDataFile("data/users/students.txt", [this](const auto& parts) {
        if (parts.size() == 4) {
            int id = stoi(parts[0]);
            auto student = make_unique<Student>(id, parts[1], parts[2]);
            student->setDepartment(parts[3]);
            addUser(move(student));
            loadAccountInfo(id);
            cout << "Loaded student: " << parts[1] << " (ID: " << id << ")" << endl;
        }
    });

    cout << "Loading professors..." << endl;
    readDataFile("data/users/professors.txt", [this](const auto& parts) {
        if (parts.size() == 4) {
            int id = stoi(parts[0]);
            auto professor = make_unique<Professor>(id, parts[1], parts[2]);
            professor->setDepartment(parts[3]);
            addUser(move(professor));
            loadAccountInfo(id);
            cout << "Loaded professor: " << parts[1] << " (ID: " << id << ")" << endl;
        }
    });

    cout << "Loading librarians..." << endl;
    readDataFile("data/users/librarians.txt", [this](const auto& parts) {
        if (parts.size() == 4) {
            int id = stoi(parts[0]);
            auto librarian = make_unique<Librarian>(id, parts[1], parts[2]);
            librarian->setDepartment(parts[3]);
            addUser(move(librarian));
            loadAccountInfo(id);
            cout << "Loaded librarian: " << parts[1] << " (ID: " << id << ")" << endl;
        }
    });
    cout << "State loading complete" << endl;
}

void Library::loadAccountInfo(int userID) {
    string accountPath = "data/accounts/" + to_string(userID) + ".txt";
    ifstream file(accountPath);
    if (!file.is_open()) {
        accounts[userID] = make_unique<Account>(userID);
        return;
    }

    auto account = make_unique<Account>(userID);
    string line;
    while (getline(file, line)) {
        auto parts = split(line, '|');
        if (parts.size() < 2) continue;

        if (parts[0] == "BORROW") {
            int bookID = stoi(parts[1]);
            time_t borrowTime = stoll(parts[2]);
            time_t dueTime = stoll(parts[3]);
            
            BorrowRecord record;
            record.bookID = bookID;
            record.borrowDate = chrono::system_clock::from_time_t(borrowTime);
            record.dueDate = chrono::system_clock::from_time_t(dueTime);
            
            account->addBorrow(bookID);
            
            if (auto book = const_cast<Book*>(getBook(bookID))) {
                book->setAvailable(false);
            }
        }
        else if (parts[0] == "HISTORY") {
            int bookID = stoi(parts[1]);
            time_t borrowTime = stoll(parts[2]);
            time_t dueTime = stoll(parts[3]);
            
            BorrowRecord record;
            record.bookID = bookID;
            record.borrowDate = chrono::system_clock::from_time_t(borrowTime);
            record.dueDate = chrono::system_clock::from_time_t(dueTime);
            
            account->addToBorrowHistory(record);
        }
        else if (parts[0] == "FINE") {
            double fine = stod(parts[1]);
            account->addFine(fine);
        }
    }
    
    accounts[userID] = move(account);
    file.close();
}

vector<string> Library::split(const string& str, char delim) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

vector<BorrowInfo> Library::getAllBorrowedBooks() const {
    vector<BorrowInfo> borrowedBooks;
    
    for (const auto& pair : accounts) {
        const Member* user = getMember(pair.first);
        if (!user) continue;
        
        for (const auto& borrow : pair.second->getCurrentBorrows()) {
            const Book* book = getBook(borrow.bookID);
            if (!book) continue;
            
            borrowedBooks.push_back({
                book,
                user,
                borrow.borrowDate,
                borrow.dueDate
            });
        }
    }
    return borrowedBooks;
}
