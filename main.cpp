#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "include/json.hpp"

using json = nlohmann::json;
using namespace std;

class DateUtils {
public:
    static string today(){
        auto now = chrono::system_clock::now();
        time_t currentTime =chrono::system_clock::to_time_t(now);
        tm localTime{};
        localTime = *localtime(&currentTime);
        stringstream ss;
        ss << put_time(&localTime, "%Y-%m-%d");
        return ss.str();
    };
    static string addDays(const string& date,int days){
        tm time = {};
        stringstream ss(date);
        ss >> get_time(&time, "%Y-%m-%d");
        if (ss.fail()) return "";
        time.tm_mday += days;
        mktime(&time);
        stringstream result;
        result << put_time(&time, "%Y-%m-%d");
        return result.str();
    };
    static bool isAfter(const string& date1, const string& date2){
        return date1 > date2;
    }
};

class File {
private:
    string filename;
    string registeredOn;
    string expiresOn;
public:
    File(string name, string registered, string expires) {
        filename = name;
        registeredOn = registered;
        expiresOn = expires;
    }
    string getFilename() const {
        return filename;
    };
    string getRegisteredOn() const {
        return registeredOn;
    };
    string getExpiresOn() const {
        return expiresOn;
    };
    bool isExpired(const string& today) const {
        return DateUtils::isAfter(today, expiresOn);
    }
};

class MetaDataStore {
private:
    string metadataPath;
public:
    MetaDataStore(const string& path) {
        metadataPath = path;
    }
    vector<File> loadAll(){
        vector<File> files;
        ifstream in(metadataPath);
        if (!in) return files;
        try {
            json data;
            in >> data;
            if (!data.is_array()) return files;
            for (const auto& item : data) {
                files.emplace_back(
                    item.at("filename").get<string>(),
                    item.at("registeredOn").get<string>(),
                    item.at("expiresOn").get<string>()
                );
            }
        }
        catch (const exception&) {
            return {};
        }
        return files;
    };
    bool saveAll(const vector<File>& files){
        json data = json::array();
        for (const File& file : files) {
            data.push_back({
                {"filename", file.getFilename()},
                {"registeredOn", file.getRegisteredOn()},
                {"expiresOn", file.getExpiresOn()}
            });
        }
        ofstream out(metadataPath);
        if (!out) return false;

        try {
            out << data.dump(4);
            return out.good();
        }
        catch (const exception&) {
            return false;
        }
    };
    bool contains(const string& filename){
        vector<File> files = loadAll();
        for (const File& file : files) {
            if (file.getFilename() == filename) {
                return true;
            }
        }
        return false;
    };
};

class FileManager {
private:
    string managedDirectory;
public:
    FileManager(const string& directory) {
        managedDirectory = directory;
    }
    vector<string> listPDFs() {
        vector<string> pdfs;
        string searchPath = managedDirectory + "\\*.pdf";
        WIN32_FIND_DATAA fileData;
        HANDLE handle = FindFirstFileA(
            searchPath.c_str(), &fileData
        );
        if (handle == INVALID_HANDLE_VALUE) return pdfs;
        do {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                pdfs.push_back(fileData.cFileName);
            }
        } while (FindNextFileA(handle, &fileData));

        FindClose(handle);
        return pdfs;
    }
    bool exists(const string& filename) {
        string path = managedDirectory + "\\" + filename;
        DWORD attributes = GetFileAttributesA(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
    }
    bool deletePDF(const string& filename) {
        if (filename.find_first_of("\\/:") != string::npos ||filename == "." || filename == "..") return false;
        string path = managedDirectory + "\\" + filename;
        DWORD attributes = GetFileAttributesA(path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
        return DeleteFileA(path.c_str()) != 0;
    }
};

class PDFManager {
private:
    MetaDataStore metadataStore;
    FileManager fileManager;
public:
    PDFManager(const string& directory, const string& metadataPath): metadataStore(metadataPath), fileManager(directory) {};
    void initialize(){
        registerUntrackedPDFs();
        cleanupExpiredPDFs();
    };
    void registerPDF(const string& filename) {
        if (!fileManager.exists(filename)) {
            cout << "PDF does not exist: " << filename << endl;
            return;
        }
        vector<File> files = metadataStore.loadAll();
        for (const File& file : files) {
            if (file.getFilename() == filename) {
                cout << "PDF already registered: " << filename << endl;
                return;
            }
        }
        string registeredOn = DateUtils::today();
        string expiresOn = DateUtils::addDays(registeredOn, 15);
        File newFile(filename, registeredOn, expiresOn);
        files.push_back(newFile);
        if (metadataStore.saveAll(files)) {
            cout << "PDF registered successfully: " << filename << endl;
        } else {
            cout << "Failed to save metadata for: " << filename << endl;
        }
    }
    void registerUntrackedPDFs() {
        vector<string> pdfs = fileManager.listPDFs();
        for (const string& filename : pdfs) {
            if (!metadataStore.contains(filename)) {
                registerPDF(filename);
            }
        }
    }
    void cleanupExpiredPDFs() {
        vector<File> files = metadataStore.loadAll();
        vector<File> remainingFiles;

        string today = DateUtils::today();

        for (const File& file : files) {
            if (file.isExpired(today)) {
                if (!fileManager.exists(file.getFilename()) ||
                    fileManager.deletePDF(file.getFilename())) {
                    cout << "Expired PDF removed: "
                        << file.getFilename() << endl;
                } else {
                    remainingFiles.push_back(file);
                    cout << "Failed to delete: "
                        << file.getFilename() << endl;
                }
            } else {
                remainingFiles.push_back(file);
            }
        }

        if (!metadataStore.saveAll(remainingFiles)) {
            cout << "Failed to update metadata!" << endl;
        }
    }
};

int main() {
    string directory = "PDFs";
    string metadataPath = "metadata.json";
    PDFManager manager(directory, metadataPath);
    manager.initialize();
    int choice;
    string filename;
    do {
        cout << "\n===== PDF Manager =====\n";
        cout << "1. Register PDF\n";
        cout << "2. Cleanup expired PDFs\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;
        switch (choice) {
            case 1:
                cout << "Enter PDF filename: ";
                cin >> filename;
                manager.registerPDF(filename);
                break;
            case 2:
                manager.cleanupExpiredPDFs();
                break;
            case 3:
                cout << "Exiting PDF Manager.\n";
                break;
            default:
                cout << "Invalid choice!\n";
        }
    } while (choice != 3);
    return 0;
}