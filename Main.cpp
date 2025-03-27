#include <iostream>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

class User {
public:
    string name;
    vector<string> borrowed_books;
    static const int BORROW_LIMIT = 5;

    User(string n) : name(n) {}

    bool can_borrow() {
        return borrowed_books.size() < BORROW_LIMIT;
    }

    void borrow_book(string isbn) {
        if (can_borrow()) {
            borrowed_books.push_back(isbn);
        } else {
            cout << name << " has reached the borrow limit!" << endl;
        }
    }

    void return_book(string isbn) {
        for (auto it = borrowed_books.begin(); it != borrowed_books.end(); ++it) {
            if (*it == isbn) {
                borrowed_books.erase(it);
                return;
            }
        }
        cout << name << " did not borrow the book with ISBN: " << isbn << endl;
    }

    void display_borrowed_books() {
        if (borrowed_books.empty()) {
            cout << name << " has no borrowed books." << endl;
        } else {
            cout << name << " has borrowed the following books:" << endl;
            for (const auto& isbn : borrowed_books) {
                cout << isbn << endl;
            }
        }
    }
};

class Book {
public:
    string title;
    string author;
    string isbn;
    string genre;
    bool is_borrowed;

    Book(string t, string a, string i, string g) {
        title = t;
        author = a;
        isbn = i;
        genre = g;
        is_borrowed = false;
    }

    void display() {
        cout << "Title: " << title << ", Author: " << author
             << ", ISBN: " << isbn << ", Genre: " << genre << endl;
    }

    string to_string() const {
        return title + "|" + author + "|" + isbn + "|" + genre;
    }

    static Book from_string(const string& data) {
        stringstream ss(data);
        string title, author, isbn, genre;
        getline(ss, title, '|');
        getline(ss, author, '|');
        getline(ss, isbn, '|');
        getline(ss, genre, '|');
        return Book(title, author, isbn, genre);
    }
};

class Library {
private:
    unordered_map<string, Book*> books;
    unordered_map<string, vector<Book*>> genre_tree;
    queue<string> borrow_queue;
    unordered_map<string, string> borrowed_books;
    unordered_map<string, User*> users;

public:
    void add_book(Book* book) {
        books[book->isbn] = book;
        genre_tree[book->genre].push_back(book);
        save_books_to_file("books.txt");  // Save after adding book
    }

    void add_user(User* user) {
        users[user->name] = user;
        save_users_to_file("users.txt");  // Save after adding user
    }

    void display_books() {
        if (books.empty()) {
            cout << "No books in the library!" << endl;
        } else {
            for (auto& pair : books) {
                pair.second->display();
            }
        }
    }

    void display_books_by_genre(string genre) {
        if (genre_tree.find(genre) != genre_tree.end()) {
            cout << "Books in genre: " << genre << endl;
            for (auto& book : genre_tree[genre]) {
                book->display();
            }
        } else {    
            cout << "No books found in this genre!" << endl;
        }
    }

    void borrow_book(string isbn, string username) {
        if (books.find(isbn) != books.end() && users.find(username) != users.end()) {
            Book* book = books[isbn];
            User* user = users[username];

            if (book->is_borrowed) {
                borrow_queue.push(username);
                cout << "Book is currently borrowed. " << username << " added to the queue." << endl;
            } else {
                if (user->can_borrow()) {
                    book->is_borrowed = true;
                    borrowed_books[isbn] = username;
                    user->borrow_book(isbn);
                    cout << username << " successfully borrowed: " << book->title << endl;
                    save_books_to_file("books.txt");  // Save after borrowing book
                } else {
                    cout << username << " has reached the borrow limit!" << endl;
                }
            }
        } else {
            cout << "Book or User not found!" << endl;
        }
    }

    void return_book(string isbn, string username) {
        if (borrowed_books.find(isbn) != borrowed_books.end() && borrowed_books[isbn] == username) {
            Book* book = books[isbn];
            User* user = users[username];
            book->is_borrowed = false;
            borrowed_books.erase(isbn);
            user->return_book(isbn);
            cout << username << " returned the book: " << book->title << endl;

            if (!borrow_queue.empty()) {
                string next_user = borrow_queue.front();
                borrow_queue.pop();
                borrow_book(isbn, next_user);
            }
            save_books_to_file("books.txt");  // Save after returning book
            save_users_to_file("users.txt");  // Save user data after returning book
        } else {
            cout << "Book was not borrowed by " << username << endl;
        }
    }

    void display_user_borrowed_books(string username) {
        if (users.find(username) != users.end()) {
            users[username]->display_borrowed_books();
        } else {
            cout << "User not found!" << endl;
        }
    }

    bool is_book_available(string isbn) {
        if (books.find(isbn) != books.end()) {
            return !books[isbn]->is_borrowed;
        }
        return false;
    }

    void load_books_from_file(const string& filename) {
        ifstream file(filename);
        if (!file) return;
        string line;
        while (getline(file, line)) {
            Book book = Book::from_string(line);
            add_book(new Book(book.title, book.author, book.isbn, book.genre));
        }
        file.close();
    }

    void save_books_to_file(const string& filename) {
        ofstream file(filename);
        for (auto& pair : books) {
            file << pair.second->to_string() << endl;
        }
        file.close();
    }

    void load_users_from_file(const string& filename) {
        ifstream file(filename);
        if (!file) return;
        string line;
        while (getline(file, line)) {
            add_user(new User(line));
        }
        file.close();
    }

    void save_users_to_file(const string& filename) {
        ofstream file(filename);
        for (auto& pair : users) {
            file << pair.first << endl;
        }
        file.close();
    }
};

void display_menu() {
    cout << "\nLibrary Management System\n";
    cout << "1. Add Book\n";
    cout << "2. Add User\n";
    cout << "3. Display All Books\n";
    cout << "4. Display Books by Genre\n";
    cout << "5. Borrow Book\n";
    cout << "6. Return Book\n";
    cout << "7. Display Borrowed Books by User\n";
    cout << "8. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    Library library;

    // Load previously saved data (if exists)
    library.load_books_from_file("books.txt");
    library.load_users_from_file("users.txt");

    int choice;
    while (true) {
        display_menu();
        cin >> choice;

        if (choice == 1) {
            string title, author, isbn, genre;
            cout << "Enter Book Title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Enter Author: ";
            getline(cin, author);
            cout << "Enter ISBN: ";
            getline(cin, isbn);
            cout << "Enter Genre: ";
            getline(cin, genre);
            library.add_book(new Book(title, author, isbn, genre));
            cout << "Book added successfully.\n";
        } else if (choice == 2) {
            string username;
            cout << "Enter Username: ";
            cin >> username;
            library.add_user(new User(username));
            cout << "User added successfully.\n";
        } else if (choice == 3) {
            library.display_books();
        } else if (choice == 4) {
            string genre;
            cout << "Enter Genre: ";
            cin.ignore();
            getline(cin, genre);
            library.display_books_by_genre(genre);
        } else if (choice == 5) {
            string isbn, username;
            cout << "Enter ISBN of Book: ";
            cin >> isbn;
            cout << "Enter Username: ";
            cin >> username;
            library.borrow_book(isbn, username);
        } else if (choice == 6) {
            string isbn, username;
            cout << "Enter ISBN of Book: ";
            cin >> isbn;
            cout << "Enter Username: ";
            cin >> username;
            library.return_book(isbn, username);
        } else if (choice == 7) {
            string username;
            cout << "Enter Username: ";
            cin >> username;
            library.display_user_borrowed_books(username);
        } else if (choice == 8) {
            cout << "Exiting Library Management System...\n";

            // Save data before exiting
            library.save_books_to_file("books.txt");
            library.save_users_to_file("users.txt");

            break;
        } else {
            cout << "Invalid choice! Please try again.\n";
        }
    }

    return 0;
}

