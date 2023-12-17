#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <ctime> // Для работы с временем
#include <chrono> // Для преобразования даты-времени
#include <iomanip> // Для форматирования вывода даты-времени
#pragma warning(disable : 4996)

using namespace std;

string read_ini_file(const string& filename, const string& key) {
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        if (line.find(key) != string::npos) {
            size_t start = line.find('=') + 1;
            size_t end = line.length();
            return line.substr(start, end - start);
        }
    }

    return "";
}

struct Client {
    string fullName;
    string phoneNumber;
    string startDate;
    string endDate;
    int debt;
    int creditLimit;
};

struct Service {
    string name;
    int code;
    double rate;
    string interval;
};

struct Usage {
    string phoneNumber;
    int serviceCode;
    string dateTime;
    int durationSeconds;
};

int SecondsInMonth() {
    //Cчитаем сколько секунд в месяце
    int month;
    int yearday;
    time_t now = time(NULL);
    tm* localTime = localtime(&now);
    month = localTime->tm_mon + 1;
    yearday = localTime->tm_yday + 1;
    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) {
        return (2678400);
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return (2592000);
    }
    else {
        if (yearday == 365) {
            return (2419200);
        }
        else {
            return (2505600);
        }
    }
}

bool IsDateInCurrentMonth(const string& dateTime) {
    // Получаем текущую дату и время
    time_t now = time(NULL);
    tm* localTime = localtime(&now);

    // Парсим dateTime
    istringstream ss(dateTime);
    int year, month, day, hour, minute, second;
    char delimiter;
    ss >> year >> delimiter >> month >> delimiter >> day >> hour >> delimiter >> minute >> delimiter >> second;

    // Если месяцы совпадают, возвращаем true, иначе false
    return (month == localTime->tm_mon + 1); // tm_mon возвращает месяц от 0 до 11
}

int main() {
    vector<Client> clients;
    vector<Service> services;
    vector<Usage> usages;
    vector<int> range;
    map<int, double> serviceRates;
    //setlocale(LC_ALL, "Russian");

    // Чтение информации о клиентах
    ifstream clientsFile("Clients.txt");
    if (clientsFile.is_open()) {
        string line;
        while (getline(clientsFile, line)) {
            istringstream ss(line);
            string token;
            Client client;
            for (int i = 0; getline(ss, token, ','); ++i) {
                switch (i) {
                case 0: client.fullName = token; break;
                case 1: client.phoneNumber = stod(token); break;
                case 2: client.startDate = token; break;
                case 3: client.endDate = token; break;
                case 4: client.debt = stoi(token); break;
                case 5: client.creditLimit = stoi(token); break;
                }
            }
            clients.push_back(client);
        }
        clientsFile.close();
    }
    else {
        cout << "Error: Unable to open clients file." << endl;
        return 1;
    }


    // Чтение информации об услугах
    ifstream servicesFile("Services.txt");

    if (servicesFile.is_open()) {
        string line;
        while (getline(servicesFile, line)) {
            istringstream ss(line);
            string token;
            Service service;
            for (int i = 0; getline(ss, token, ','); ++i) {
                switch (i) {
                case 0: service.name = token; break;
                case 1: service.code = stoi(token); break;
                case 2: service.rate = stod(token); break;  // Используем stod для считывания значения с плавающей точкой
                case 3: service.interval = token; break;
                }
            }
            services.push_back(service);
        }
        servicesFile.close();
    }
    else {
        cout << "Error: Unable to open services file." << endl;
        return 1;
    }


    // Чтение информации об использовании услуг
    ifstream usageFile("Usage.txt");
    if (usageFile.is_open()) {
        string line;
        while (getline(usageFile, line)) {
            istringstream ss(line);
            string token;
            Usage usage;
            for (int i = 0; getline(ss, token, ','); ++i) {
                switch (i) {
                case 0: usage.phoneNumber = stod(token); break;
                case 1: usage.serviceCode = stoi(token); break;
                case 2: usage.dateTime = token; break;
                case 3: usage.durationSeconds = stoi(token); break;
                }
            }
            if (IsDateInCurrentMonth(usage.dateTime)) { // Проверяем, что дата в текущем месяце
                usages.push_back(usage);
            }
        }
        usageFile.close();
    }
    else {
        cout << "Error: Unable to open usage file." << endl;
        return 1;
    }

    // Чтение диапазона параметров.
    string filename = "Param.ini";
    string key1 = "key1";
    string key2 = "key2";
    string value1 = read_ini_file(filename, key1);
    string value2 = read_ini_file(filename, key2);

    // Создание словаря тарифов услуг
    for (const auto& service : services) {
        double modifiedRate = service.rate; // Значение по умолчанию

        // Изменяем тариф в зависимости от интервала
        if (service.interval == " мин") {
            modifiedRate /= 60;
        }
        else if (service.interval == " час") {
            modifiedRate /= 3600; 
        }
        else if (service.interval == " сутки") {
            modifiedRate /= 86400; 
        }
        else if (service.interval == " месяц") {

            modifiedRate /= SecondsInMonth(); 
        }
        else {
            modifiedRate /= 1; // Если интервал не определен, используем значение по умолчанию
        }
        // Внесем скорректированный тариф в словарь serviceRates
        serviceRates[service.code] = modifiedRate;
    }

    // Вычисление потраченной суммы.
    ofstream reportFile("Report.txt");
    bool flag = false;
    map<int, double> totalCostPerService;
    for (const auto& usage : usages) {
        for (const auto& client : clients) {
            if (client.phoneNumber == usage.phoneNumber) {
                auto it = serviceRates.find(usage.serviceCode);
                if (it != serviceRates.end()) {
                    double cost = it->second * usage.durationSeconds;
                    if (cost >= stod(value1) && cost <= stod(value2)) {
                        totalCostPerService[usage.serviceCode] += cost;
                    }
                }
            }
        }
    }


    for (const auto& pair : totalCostPerService) {
        // Ищем название сервиса по его коду
        string serviceName;
        for (const auto& service : services) {
            if (service.code == pair.first) {
                serviceName = service.name;
                break;
            }
        }
        flag = true;
        reportFile << "Сервис: " << serviceName << " (" << pair.first << "), Сумма: " << pair.second << " рублей" << endl;
    }

    if (flag) {
        cout << "Report created successfully!" << endl;
    }
    else {
        cout << "The report was not created!" << endl;
    }

    reportFile.close();

    return 0;
}