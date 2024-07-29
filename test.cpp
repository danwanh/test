
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <queue>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
//#pragma comment(lib, "ws2_32.lib")
#include <cstring>
#include <iostream>
//#include <filesystem>
#include <cstdint>


using namespace std;
using namespace chrono;
// namespace fs = std::filesystem;

// functions, classes, or variables here




enum class FileDowloadPriority
{
    LOW,
    NORMAL,
    HIGH,
    CRITICAL,
    NONE
};

class File
{
private:
    string name = "";
    int64_t size = 0;
    FileDowloadPriority priority = FileDowloadPriority::NONE;
    int dowloadProcess = 0;
public:
    string getName();

    int64_t getSize();

    FileDowloadPriority getPriority();

    void setPriority(FileDowloadPriority priority);

    string getPriorityString();

    void setName(string name);

    void setSize(int64_t size);

    char* serialize(int& buffer_size);

    void deserialize(char* buffer);
};



// Getter and Setter
string File::getName()
{
    return name;
}

void File::setName(string name)
{
    this->name = name;
}

int64_t File::getSize()
{
    return size;
}

void File::setSize(int64_t size)
{
    this->size = size;
}

FileDowloadPriority File::getPriority()
{
    return priority;
}

void File::setPriority(FileDowloadPriority priority)
{
    this->priority = priority;
}

// Serialize File object to a buffer
char* File::serialize(int& buffer_size)
{

    int nameSize = this->name.size();
    buffer_size = sizeof(int) + sizeof(nameSize) + nameSize + sizeof(this->size) + sizeof(this->priority);

    char* buffer = new char[buffer_size];
    // copy data to buffer
    int offset = 0;
    // copy buffer size to buffer
    memcpy(buffer + offset, &buffer_size, sizeof(buffer_size));
    offset += sizeof(buffer_size);
    // copy name size to buffer
    memcpy(buffer + offset, &nameSize, sizeof(nameSize));
    offset += sizeof(nameSize);
    // copy name to buffer
    memcpy(buffer + offset, this->name.c_str(), nameSize);
    offset += nameSize;
    // copy size to buffer
    memcpy(buffer + offset, &this->size, sizeof(this->size));
    offset += sizeof(this->size);
    // copy priority to buffer
    memcpy(buffer + offset, &this->priority, sizeof(this->priority));
    return buffer;
}

void File::deserialize(char* buffer)
{
    int offset = 0;
    int nameSize;
    // copy name size from buffer
    memcpy(&nameSize, buffer + offset, sizeof(nameSize));
    offset += sizeof(nameSize);
    string name(buffer + offset, nameSize);
    // copy name from buffer
    offset += nameSize;
    int64_t size;
    memcpy(&size, buffer + offset, sizeof(size));
    // copy size from buffer
    offset += sizeof(size);
    FileDowloadPriority priority;
    memcpy(&priority, buffer + offset, sizeof(priority));

    this->name = name;
    this->size = size;
    this->priority = priority;
}

string File::getPriorityString()
{
    switch (this->priority)
    {
    case FileDowloadPriority::LOW:
        return "LOW";
    case FileDowloadPriority::NORMAL:
        return "NORMAL";
    case FileDowloadPriority::HIGH:
        return "HIGH";
    case FileDowloadPriority::CRITICAL:
        return "CRITICAL";
    default:
        return "NONE";
    }
}




class FileService
{
private:
    vector<File> fileArr;
public:
    FileService();
    ~FileService();

    //  void setFileArr();
    vector<File>& getFileArr();

    void sendFileArr(SOCKET clientSocket);
    void receiveFileArr(SOCKET serverSocket);

    char* serializeFileArr(int& buffer_size);
    vector<File> deserializeFileArr(char* buffer, int buffer_size);

    void readUserInput(string fileName);
};

FileService::FileService()
{
}
FileService::~FileService()
{
}

char* FileService::serializeFileArr(int& buffer_size)
{
    vector<char*> serializedFilesArr;
    int fileCount = this->fileArr.size();

    int bufferFileSize = 0;

    for (int i = 0; i < fileCount; i++)
    {
        char* serializedFile = this->fileArr[i].serialize(bufferFileSize);
        serializedFilesArr.push_back(serializedFile);
        buffer_size += bufferFileSize;
    }

    char* buffer = new char[buffer_size];
    int offset = 0;

    // n?i l?i
    for (int i = 0; i < fileCount; i++)
    {
        int bufferFileSize = 0;
        std::memcpy(&bufferFileSize, serializedFilesArr[i], sizeof(bufferFileSize));
        std::memcpy(buffer + offset, serializedFilesArr[i], bufferFileSize);
        offset += bufferFileSize;
        delete[] serializedFilesArr[i];
    }
    return buffer;
}

vector<File>& FileService::getFileArr()
{
    return this->fileArr;
}

std::vector<File> FileService::deserializeFileArr(char* buffer, int buffer_size)
{
    std::vector<File> files;
    int offset = 0;

    while (offset < buffer_size)
    {
        int bufferFileSize = 0;
        std::memcpy(&bufferFileSize, buffer + offset, sizeof(bufferFileSize));
        offset += sizeof(bufferFileSize);

        File file;
        file.deserialize(buffer + offset);
        files.push_back(file);
        offset += bufferFileSize - sizeof(int);
    }
    return files;
}


void FileService::readUserInput(string fileName)
{
    fileArr.resize(0);
    fstream fi;
    fi.open(fileName.c_str(), ios::in);
    if (!fi.is_open())
    {
        cout << " Can not open file ";
        return;
    }
    string line;
    File file;
    while (getline(fi, line))
    {
        stringstream ss(line);
        string s;
        ss >> s;
        file.setName(s);
        ss >> s;
        if (s == "LOW")
            file.setPriority(FileDowloadPriority::LOW);
        else if (s == "NORMAL")
            file.setPriority(FileDowloadPriority::NORMAL);
        else if (s == "HIGH")
            file.setPriority(FileDowloadPriority::HIGH);
        else if (s == "CRITICAL")
            file.setPriority(FileDowloadPriority::CRITICAL);
        else
            file.setPriority(FileDowloadPriority::NONE);

        fileArr.push_back(file);
    }
    fi.close();
}




struct FileProcess
{
    File file;
    int process = 0;

    FileProcess(File file, int process)
    {
        this->file = file;
        this->process = process;
    }

    string getName()
    {
        return this->file.getName();
    }

    File getFile()
    {
        return file;
    }
};

class Controller
{
public:
    Controller(SOCKET socket);
    ~Controller();
    void run();
    void updateFileQueue(FileService requestFile);
    char* serializeData(struct  FileProcess, int& bufferSize);
private:
    vector<FileProcess> fileQueue;
    SOCKET socket;
    // Declare any private member variables or functions here
};
// Declare your class or function prototypes here

Controller::Controller(SOCKET socket)
{
    this->socket = socket;
}
Controller::~Controller()
{
}


void Controller::updateFileQueue(FileService requestFile)
{
    // update the queue download file
    int size = requestFile.getFileArr().size();
    int sizeQueue = fileQueue.size();
    for (int i = 0; i < size; i++)
    {

        // check if the file is already in the queue
        bool isExist = false;
        for (int j = 0; j < sizeQueue; j++)
        {
            if (requestFile.getFileArr()[i].getName() == this->fileQueue[j].getName())
            {
                //  update the file in the queue
                this->fileQueue[j].file = requestFile.getFileArr()[i];
                isExist = true;
            }
        }
        if (!isExist)
        {
            // add the file to the queue
            FileProcess fileProcess(requestFile.getFileArr()[i], 0);
            this->fileQueue.push_back(fileProcess);
        }
    }
}


//////////////////////////////////////////////////////////////////////////


char* readData(FileProcess fileProcess, int& dataSize) {
    fstream fi;
    string fileName = fileProcess.getName();
    fi.open(fileName.c_str(), ios::in | ios::binary);
    if (fi.fail())
        return NULL;

    // tính kích thước file
    fi.seekg(0, ios::end);
    int file_size = fi.tellg();

    // Lấy dữ dataSize của gói tin thứ " process"
    int process = fileProcess.process;
    if ((process + 1) * 512 >= file_size) {
        dataSize = file_size - process * 512;
        if (dataSize >= 512)
            dataSize = 0;
    }
    else
        dataSize = 512;
    char* data = new char[dataSize];

    // Di chuyển tới vị trí đầu tiên cần lấy dữ liệu trong file
    fi.seekg(process * 512, ios::beg);
    fi.read(data, dataSize);

    fi.close();
    return data;
}

// Lưu dữ liệu vào char* bufer
char* serializeData(FileProcess fileProcess, int& bufferSize) {

    string fileName = fileProcess.getName();
    int fileNameLength = fileName.size();
    int process = fileProcess.process;
    int dataSize;
     
    // Lấy data và dataSize
    char* data = readData(fileProcess, dataSize);

    bufferSize = sizeof(fileNameLength) + fileNameLength + sizeof(process)
        + sizeof(dataSize) + dataSize;

    char* buffer = new char[bufferSize];
    int offset = 0;

    // lưu độ dài fileName
    memcpy(buffer + offset, &fileNameLength, sizeof(fileNameLength));
    offset += sizeof(fileNameLength);

    // lưu fileName
    memcpy(buffer + offset, fileName.c_str(), fileNameLength);
    offset += fileNameLength;

    // Lưu biến process
    memcpy(buffer + offset, &process, sizeof(process));
    offset += sizeof(process);

    // Lưu dataSize
    memcpy(buffer + offset, &dataSize, sizeof(dataSize));
    offset += sizeof(dataSize);

    // Lưu Data
    memcpy(buffer + offset, data, dataSize);

    return buffer;
}


void derializeData(char* bufferData) {

    int offset = 0;

    // Lây độ dài fileName
    int fileNameLength;
    memcpy(&fileNameLength, bufferData, sizeof(fileNameLength));

    // Lấy fileName
    offset += sizeof(fileNameLength);
    char* fileName = new char[fileNameLength];
    memcpy(fileName, bufferData + offset, fileNameLength);

    // Mở file để ghi dữ liệu lên. tên file là tên của fileName
    // Do fileName trùng với tên của file cần tải nên tui bỏ dữ liệu vào file tên output.txt nha
 
 
    ofstream fo;
    fo.open("output.txt", ios::binary | ios::app);
    if (fo.fail()) {
       delete[] fileName;
        return;
    }
     
    offset += fileNameLength;
    int process;
    memcpy(&process, bufferData + offset, sizeof(process));

    offset += sizeof(process);
    int dataSize;
    memcpy(&dataSize, bufferData + offset, sizeof(dataSize));

    offset += sizeof(dataSize);
    char* data = new char[dataSize];
    memcpy(data, bufferData + offset, dataSize);

    // Ghi dữ liệu vào cuối file
    fo.write(data, dataSize);
    fo.close();

   // delete[] fileName;
    delete[] data;
}


int main() {

   // Tạo file tên test.dat ; file này đóng vai trò là file cần truyền cho client
    ofstream f("test.dat", ios::binary);

    string A = " How to be brave";
    int B = 123;
    f.write((char*)&B, sizeof(B));
    f.write(A.c_str(), A.size());
    f.seekp(0, ios::end);
    int dataSize = f.tellp();
    cout << " Data Size of file test.dat is: " << dataSize << endl;
    f.close();


    // Tạo đối tượng File file, trong đó tên file cần tải là "test.data"
    File file;
    file.setName("test.dat");

    FileProcess fileProcess(file, 0);

    int bufferSize;
    
    // Check hàm viết ở trên; lưu dữ liệu của gói tin vào char* buffer
    char* buffer = serializeData(fileProcess, bufferSize);

    cout << " Buffer Size " << bufferSize << endl;

    // Chuyển dữ liệu từ char* bufer ra và lưu vào một file mới có tên "output.txt"
    derializeData(buffer);

    //
    //// Check xem dữ liệu từ file "output.txt" có đúng không

    ifstream fo;
    fo.open("output.txt");
    if (fo.fail())
        cout << " Can't find result ";

    // Lấy kích thước của file "output.txt"
    fo.seekg(0, ios::end);
    int fileSize = fo.tellg();
    cout << " File Size " << fileSize << endl;

    // Kiểm tra dũ liệu có giống với file "test.dat" không
    int x;
    char* chr = new char[fileSize - 4];

    fo.seekg(0, ios::beg);
    fo.read((char*)&x, sizeof(x));
    fo.read(chr, fileSize - 4);
    // chr[fileSize - 4] = 0;
 
    cout << " Variable x has value: " << x << endl;
    cout << " Char chr has value: " << chr << endl;
    fo.close();

    delete[] chr;
    delete[] buffer;

    return 0;
}
