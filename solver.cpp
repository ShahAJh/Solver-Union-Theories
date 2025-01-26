#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

struct Term {
    string name;
    int parent;
    int rank;
    unordered_set<int> ccpar;

    Term(string n, int id) : name(n), parent(id), rank(0) {}
};

struct List {
    vector<int> elements;
    bool isCyclic;

    List() : isCyclic(false) {}
};

struct Array {
    unordered_map<int, string> indexToValue;
};

struct SolverConfig {
    bool useForbiddenList = true;
    bool useUnionByCcpar = true;
    bool useNonRecursiveFind = true;
};

class CongruenceClosure {
private:
    vector<Term> terms;
    unordered_map<string, int> termMap;
    unordered_set<string> forbiddenList;
    unordered_map<string, List> lists;
    unordered_map<string, Array> arrays;
    ofstream logFile;
    SolverConfig config;

    int find(int x) {
        while (x != terms[x].parent) {
            terms[x].parent = terms[terms[x].parent].parent;
            x = terms[x].parent;
        }
        return x;
    }

    void unionSets(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);

        if (rootX != rootY) {
            logFile << "Merging sets: " << terms[rootX].name << " and " << terms[rootY].name << endl;

            if (config.useUnionByCcpar) {
                if (terms[rootX].rank > terms[rootY].rank ||
                    (terms[rootX].rank == terms[rootY].rank && terms[rootX].ccpar.size() > terms[rootY].ccpar.size())) {
                    terms[rootY].parent = rootX;
                    terms[rootX].ccpar.insert(terms[rootY].ccpar.begin(), terms[rootY].ccpar.end());
                } else {
                    terms[rootX].parent = rootY;
                    if (terms[rootX].rank == terms[rootY].rank) {
                        terms[rootY].rank++;
                    }
                    terms[rootY].ccpar.insert(terms[rootX].ccpar.begin(), terms[rootX].ccpar.end());
                }
            } else {
                if (terms[rootX].rank > terms[rootY].rank) {
                    terms[rootY].parent = rootX;
                } else {
                    terms[rootX].parent = rootY;
                    if (terms[rootX].rank == terms[rootY].rank) {
                        terms[rootY].rank++;
                    }
                }
            }
        }
    }

public:
    CongruenceClosure(const SolverConfig& cfg) : config(cfg) {
        logFile.open("solver_log.txt", ios::app);
    }

    ~CongruenceClosure() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void addTerm(const string& name) {
        if (termMap.find(name) == termMap.end()) {
            int id = terms.size();
            terms.emplace_back(name, id);
            termMap[name] = id;
        }
    }

    void addEquality(const string& term1, const string& term2) {
        if (config.useForbiddenList &&
            (forbiddenList.find(term1) != forbiddenList.end() || forbiddenList.find(term2) != forbiddenList.end())) {
            cout << "Skipping merge: " << term1 << " or " << term2 << " is in the forbidden list." << endl;
            logFile << "Skipping merge: " << term1 << " or " << term2 << " is in the forbidden list." << endl;
            return;
        }
        addTerm(term1);
        addTerm(term2);
        unionSets(termMap[term1], termMap[term2]);
    }

    void addToForbiddenList(const string& term) {
        forbiddenList.insert(term);
    }

    void setArrayElement(const string& arrayName, int index, const string& value) {
        arrays[arrayName].indexToValue[index] = value;
    }

    string getArrayElement(const string& arrayName, int index) {
        if (arrays[arrayName].indexToValue.find(index) != arrays[arrayName].indexToValue.end()) {
            return arrays[arrayName].indexToValue[index];
        }
        return "undefined";
    }

    void addList(const string& listName, const vector<string>& elements) {
        List& list = lists[listName];
        unordered_set<string> visited;

        for (const string& element : elements) {
            if (visited.find(element) != visited.end()) {
                list.isCyclic = true;
                logFile << "List " << listName << " is cyclic." << endl;
                break;
            }
            visited.insert(element);
            addTerm(element);
            list.elements.push_back(termMap[element]);
        }
    }

    void parseInput(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            logFile << "Error opening file: " << filename << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.substr(0, 4) == "add ") {
                string term1, term2;
                size_t space = line.find(' ', 4);
                term1 = line.substr(4, space - 4);
                term2 = line.substr(space + 1);
                addEquality(term1, term2);
            } else if (line.substr(0, 9) == "forbidden") {
                string term = line.substr(10);
                addToForbiddenList(term);
            } else if (line.substr(0, 3) == "set") {
                string arrayName, value;
                int index;
                size_t firstSpace = line.find(' ', 4);
                size_t secondSpace = line.find(' ', firstSpace + 1);
                arrayName = line.substr(4, firstSpace - 4);
                index = stoi(line.substr(firstSpace + 1, secondSpace - firstSpace - 1));
                value = line.substr(secondSpace + 1);
                setArrayElement(arrayName, index, value);
            } else if (line.substr(0, 4) == "list") {
                string listName = line.substr(5, line.find(' ', 5) - 5);
                vector<string> elements;
                size_t pos = line.find('[') + 1;
                size_t end = line.find(']');
                string elementsStr = line.substr(pos, end - pos);
                size_t commaPos;
                while ((commaPos = elementsStr.find(',')) != string::npos) {
                    elements.push_back(elementsStr.substr(0, commaPos));
                    elementsStr.erase(0, commaPos + 1);
                }
                elements.push_back(elementsStr);
                addList(listName, elements);
            }
        }
        file.close();
    }

    void parseSMTLIB(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening SMT-LIB file: " << filename << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.find("declare-fun") != string::npos) {
                continue;
            } else if (line.find("assert") != string::npos) {
                size_t start = line.find("(");
                size_t end = line.find_last_of(")");
                if (start != string::npos && end != string::npos && end > start) {
                    string assertion = line.substr(start + 1, end - start - 1);

                    if (assertion.find("(=") == 0) {
                        size_t space1 = assertion.find(" ", 3);
                        size_t space2 = assertion.find(" ", space1 + 1);
                        string term1 = assertion.substr(3, space1 - 3);
                        string term2 = assertion.substr(space1 + 1, space2 - space1 - 1);
                        addEquality(term1, term2);
                    }
                }
            }
        }
        file.close();
    }

    void printClasses() {
        unordered_map<int, vector<string>> classes;

        for (const auto& term : terms) {
            int root = find(term.parent);
            classes[root].push_back(term.name);
        }

        cout << "Equivalence Classes:" << endl;
        for (const auto& [root, classTerms] : classes) {
            cout << "Class " << root << ": ";
            for (const auto& name : classTerms) {
                cout << name << " ";
            }
            cout << endl;
        }

        cout << "Forbidden Terms (not part of any class): ";
        for (const auto& forbidden : forbiddenList) {
            if (termMap.find(forbidden) == termMap.end() || find(termMap[forbidden]) == termMap[forbidden]) {
                cout << forbidden << " ";
            }
        }
        cout << endl;

        cout << "Arrays:" << endl;
        for (const auto& [arrayName, array] : arrays) {
            cout << arrayName << ": ";
            for (const auto& [index, value] : array.indexToValue) {
                cout << "[" << index << "] = " << value << ", ";
            }
            cout << endl;
        }

        cout << "Lists:" << endl;
        for (const auto& [listName, list] : lists) {
            cout << listName << ": ";
            if (list.isCyclic) {
                cout << "Cyclic" << endl;
            } else {
                for (const auto& index : list.elements) {
                    cout << terms[index].name << " ";
                }
                cout << endl;
            }
        }
    }

    void logPerformance(const string& testCaseName, const milliseconds& duration) {
        cout << "[Performance] Test Case: " << testCaseName << " took " << duration.count() << " ms" << endl;
        logFile << "[Performance] Test Case: " << testCaseName << " took " << duration.count() << " ms" << endl;
    }
};

int main() {
    string folderPath = "test_cases";
    cout << "Running all test cases from folder: " << folderPath << endl;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".txt") {
            string testCaseName = entry.path().filename().string();
            cout << "Running test case: " << testCaseName << endl;

            SolverConfig config;
            config.useUnionByCcpar = true;
            CongruenceClosure solver(config);

            auto start = high_resolution_clock::now();

            if (testCaseName.find("smt") != string::npos) {
                solver.parseSMTLIB(entry.path().string());
            } else {
                solver.parseInput(entry.path().string());
            }

            solver.printClasses();

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            solver.logPerformance(testCaseName, duration);

            cout << "=====================" << endl;
        }
    }

    return 0;
}
