#include <bits/stdc++.h>
using namespace std;

// ----------------------------- Utility -----------------------------
static inline string trim(const string &s) {
    size_t start = s.find_first_not_of("\t\n\r ");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of("\t\n\r ");
    return s.substr(start, end - start + 1);
}

static inline string sanitize(const string &s) {
    string t = s;
    for (char &c : t) {
        if (c == '|' || c == '\n' || c == '\r') c = ' ';
    }
    return trim(t);
}

// ----------------------------- Domain Model -----------------------------
class Student {
public:
    int id{};
    string name;
    int age{};
    string grade;
    string section;
    string phone;

    Student() = default;

    Student(int id, string name, int age, string grade, string section, string phone)
        : id(id), name(move(name)), age(age), grade(move(grade)), section(move(section)), phone(move(phone)) {}

    string serialize() const {
        return to_string(id) + "|" + sanitize(name) + "|" + to_string(age) + "|" + sanitize(grade) + "|" + sanitize(section) + "|" + sanitize(phone);
    }

    static Student deserialize(const string &line) {
        vector<string> parts;
        string cur;
        for (char c : line) {
            if (c == '|') { parts.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        parts.push_back(cur);
        if (parts.size() != 6) throw runtime_error("Corrupt record: incorrect field count");

        Student s;
        s.id = stoi(trim(parts[0]));
        s.name = trim(parts[1]);
        s.age = stoi(trim(parts[2]));
        s.grade = trim(parts[3]);
        s.section = trim(parts[4]);
        s.phone = trim(parts[5]);
        return s;
    }
};

// ----------------------------- Validation -----------------------------
struct Validation {
    static bool validId(int id) { return id > 0; }
    static bool validAge(int age) { return age >= 3 && age <= 100; }
};

// ----------------------------- Repository -----------------------------
class StudentRepository {
    string filepath;
public:
    explicit StudentRepository(string path) : filepath(move(path)) {
        ifstream in(filepath);
        if (!in.good()) {
            ofstream out(filepath);
        }
    }

    vector<Student> loadAll() const {
        vector<Student> v;
        ifstream in(filepath);
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            try {
                v.push_back(Student::deserialize(line));
            } catch (const exception &e) {
                cerr << "[WARN] Skipping line: " << e.what() << "\n";
            }
        }
        return v;
    }

    void saveAll(const vector<Student> &v) const {
        ofstream out(filepath, ios::trunc);
        for (const auto &s : v) out << s.serialize() << "\n";
    }

    bool add(const Student &s) {
        auto v = loadAll();
        for (auto &x : v) if (x.id == s.id) return false;
        v.push_back(s);
        saveAll(v);
        return true;
    }

    Student* getById(int id) const {
        static Student result;
        auto v = loadAll();
        for (const auto &s : v) {
            if (s.id == id) {
                result = s;
                return &result;
            }
        }
        return nullptr;
    }

    bool update(int id, const Student &updated) {
        auto v = loadAll();
        bool found = false;
        for (auto &s : v) {
            if (s.id == id) {
                s = updated;
                found = true;
                break;
            }
        }
        if (found) saveAll(v);
        return found;
    }

    bool remove(int id) {
        auto v = loadAll();
        size_t before = v.size();
        v.erase(remove_if(v.begin(), v.end(), [&](const Student &s){ return s.id == id; }), v.end());
        if (v.size() != before) {
            saveAll(v);
            return true;
        }
        return false;
    }
};

// ----------------------------- UI Helpers -----------------------------
int readInt(const string &prompt) {
    while (true) {
        cout << prompt;
        string line; getline(cin, line);
        try {
            return stoi(trim(line));
        } catch (...) {
            cout << "Invalid number. Try again.\n";
        }
    }
}

string readLine(const string &prompt, bool required = true) {
    while (true) {
        cout << prompt;
        string line; getline(cin, line);
        line = trim(line);
        if (!required || !line.empty()) return line;
        cout << "This field is required. Try again.\n";
    }
}

Student inputStudentWithId(int id) {
    string name = readLine("Name: ");
    int age = readInt("Age (3..100): ");
    while (!Validation::validAge(age)) {
        cout << "Age must be between 3 and 100.\n";
        age = readInt("Age (3..100): ");
    }
    string grade = readLine("Grade/Class: ");
    string section = readLine("Section: ");
    string phone = readLine("Phone (optional): ", false);
    return Student{id, name, age, grade, section, phone};
}

void printHeader() {
    cout << left
         << setw(6) << "ID"
         << setw(22) << "Name"
         << setw(6) << "Age"
         << setw(10) << "Grade"
         << setw(10) << "Section"
         << setw(15) << "Phone"
         << "\n";
    cout << string(69, '-') << "\n";
}

void printStudent(const Student &s) {
    cout << left
         << setw(6) << s.id
         << setw(22) << s.name.substr(0,21)
         << setw(6) << s.age
         << setw(10) << s.grade.substr(0,9)
         << setw(10) << s.section.substr(0,9)
         << setw(15) << s.phone.substr(0,14)
         << "\n";
}

// ----------------------------- Application -----------------------------
class App {
    StudentRepository repo;
public:
    explicit App(const string &path) : repo(path) {}

    void run() {
        while (true) {
            cout << "\n===== School Management System =====\n";
            cout << "1) Add Student\n";
            cout << "2) View All Students\n";
            cout << "3) Search by ID\n";
            cout << "4) Update Student\n";
            cout << "5) Delete Student\n";
            cout << "0) Exit\n";
            int choice = readInt("Choose an option: ");

            switch (choice) {
                case 1: addStudent(); break;
                case 2: viewAll(); break;
                case 3: searchById(); break;
                case 4: updateStudent(); break;
                case 5: deleteStudent(); break;
                case 0: cout << "Goodbye!\n"; return;
                default: cout << "Invalid choice. Try again.\n"; break;
            }
        }
    }

private:
    void addStudent() {
        cout << "\n-- Add Student --\n";
        int id = readInt("ID (positive, unique): ");
        while (!Validation::validId(id)) {
            cout << "ID must be positive.\n";
            id = readInt("ID (positive, unique): ");
        }
        if (repo.getById(id)) {
            cout << "A student with this ID already exists.\n";
            return;
        }
        Student s = inputStudentWithId(id);
        if (repo.add(s)) cout << "Student added successfully.\n";
        else cout << "Failed to add student.\n";
    }

    void viewAll() {
        auto v = repo.loadAll();
        if (v.empty()) { cout << "No records found.\n"; return; }
        sort(v.begin(), v.end(), [](const Student &a, const Student &b){ return a.id < b.id; });
        printHeader();
        for (const auto &s : v) printStudent(s);
    }

    void searchById() {
        int id = readInt("Enter ID: ");
        Student* s = repo.getById(id);
        if (!s) cout << "Not found.\n";
        else { printHeader(); printStudent(*s); }
    }

    void updateStudent() {
        int id = readInt("Enter existing ID: ");
        Student* cur = repo.getById(id);
        if (!cur) { cout << "Not found.\n"; return; }
        printHeader();
        printStudent(*cur);

        cout << "\nEnter new values (leave blank to keep current).\n";
        string line;

        cout << "Name [" << cur->name << "]: ";
        getline(cin, line);
        string name = trim(line).empty() ? cur->name : trim(line);

        cout << "Age [" << cur->age << "]: ";
        getline(cin, line);
        int age = cur->age;
        if (!trim(line).empty()) {
            try { age = stoi(trim(line)); } catch (...) { cout << "Invalid input. Keeping old.\n"; }
            if (!Validation::validAge(age)) age = cur->age;
        }

        cout << "Grade [" << cur->grade << "]: ";
        getline(cin, line);
        string grade = trim(line).empty() ? cur->grade : trim(line);

        cout << "Section [" << cur->section << "]: ";
        getline(cin, line);
        string section = trim(line).empty() ? cur->section : trim(line);

        cout << "Phone [" << cur->phone << "]: ";
        getline(cin, line);
        string phone = trim(line).empty() ? cur->phone : trim(line);

        Student upd{id, name, age, grade, section, phone};
        if (repo.update(id, upd)) cout << "Updated successfully.\n";
        else cout << "Update failed.\n";
    }

    void deleteStudent() {
        int id = readInt("Enter ID to delete: ");
        if (repo.remove(id)) cout << "Deleted.\n";
        else cout << "Not found.\n";
    }
};

// ----------------------------- Entry Point -----------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    App app("students.db");
    app.run();
    return 0;
}
