#define WIN32_LEAN_AND_MEAN 
#include <windows.h> 
#include <winsock2.h> 
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <wincrypt.h>
#include<string.h>
#include <conio.h>
#include <sstream>
#include <vector>
#include <string>

#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <vector>

#define MAX_COMMAND_SIZE 500
#define MAX_BUFFER_SIZE 2048
#define KEY_BUF_SIZE 256
#define MIN_PATH_SIZE 5

using namespace std;

int servers_amount = 0;

typedef struct sock
{
	int s; // Сокет
	HCRYPTPROV DescCSP; // Криптопровайдер
	HCRYPTKEY DescKey; // Асимметричный ключ (RSA)
	HCRYPTKEY DescKey_imp; // Сеансовый ключ (RC4)
	HCRYPTKEY hPublicKey, hPrivateKey; // Публичный, приватный ключи
	string ip_address; 

}socketExtended;

vector<socketExtended> sockets;

void RemoveClosedServer(int server_index) {
	if (server_index >= 0 && server_index < (int)sockets.size()) {
		// Закрываем сокет если он еще открыт
		if (sockets[server_index].s != 0) {
			closesocket(sockets[server_index].s);
		}
		// Освобождаем криптографические ресурсы
		if (sockets[server_index].DescKey_imp != 0) {
			CryptDestroyKey(sockets[server_index].DescKey_imp);
		}
		if (sockets[server_index].DescKey != 0) {
			CryptDestroyKey(sockets[server_index].DescKey);
		}
		if (sockets[server_index].hPublicKey != 0) {
			CryptDestroyKey(sockets[server_index].hPublicKey);
		}
		if (sockets[server_index].hPrivateKey != 0) {
			CryptDestroyKey(sockets[server_index].hPrivateKey);
		}
		if (sockets[server_index].DescCSP != 0) {
			CryptReleaseContext(sockets[server_index].DescCSP, 0);
		}
		// Удаляем из массива
		sockets.erase(sockets.begin() + server_index);
	}
}

void ShowMenu() {
	printf("===============================================================\n");
	printf("   1 - Добавить сервер\n");
	printf("   2 - Тип и версия ОС\n");
	printf("   3 - Текущее время\n");
	printf("   4 - Время работы системы\n");
	printf("   5 - Информация о памяти\n");
	printf("   6 - Информация о дисках\n");
	printf("   7 - Права доступа к файлу/папке/ключу реестра\n");
	printf("   8 - Владелец файла/папки/ключа реестра\n");
	printf("   9 - Закрыть сессию\n");
	printf("   0 - Выход\n");
	printf("===============================================================\n");
}


void CheckActiveServers() {
	for (int i = sockets.size() - 1; i >= 0; i--) {
		if (sockets[i].s == 0) {
			// Сервер уже закрыт, удаляем его
			RemoveClosedServer(i);
		}
		else {
			// Неблокирующая проверка сокета
			fd_set read_fds;
			fd_set error_fds;
			struct timeval timeout;

			FD_ZERO(&read_fds);
			FD_ZERO(&error_fds);
			FD_SET(sockets[i].s, &read_fds);
			FD_SET(sockets[i].s, &error_fds);

			timeout.tv_sec = 0;
			timeout.tv_usec = 1000; // 1 миллисекунда

			int result = select(0, &read_fds, NULL, &error_fds, &timeout);

			if (result == SOCKET_ERROR) {
				// Ошибка сокета - соединение разорвано
				cout << "Соединение с сервером " << sockets[i].ip_address << " разорвано (ошибка сокета)." << endl;
				RemoveClosedServer(i);
			}
			else if (result > 0) {
				if (FD_ISSET(sockets[i].s, &error_fds)) {
					// Ошибка на сокете - соединение разорвано
					cout << "Соединение с сервером " << sockets[i].ip_address << " разорвано." << endl;
					RemoveClosedServer(i);
				}
			}
			// Если result == 0, сокет активен, но данных нет - это нормально
		}
	}
}

void ShowServers() {
	CheckActiveServers();
	printf("\n=== ПОДКЛЮЧЕННЫЕ СЕРВЕРЫ ===\n");
	if (sockets.empty()) {
		printf("   Нет подключенных серверов\n");
	}
	else {
		for (size_t i = 0; i < sockets.size(); i++) {
			printf("   %d - Сервер %s\n", i + 1, sockets[i].ip_address.c_str());
		}
	}
	printf("\n===============================================================\n");
}

int init()
{
	WSADATA wsa_data;
	return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data));
}

void s_close(int s)
{
	closesocket(s);
}

int sock_err(const char* function, int s)
{
	int err;
	err = WSAGetLastError();
	fprintf(stderr, "%s: ошибка сокета: %d\n", function, err);
	return -1;
}

int connect_200ms(int s, struct sockaddr_in addr)
{
	for (int rec = 0; rec < 5; rec++)
	{
		if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == 0)
			return 0;
		else
		{
			fprintf(stdout, "%i попытка подключения не удалась\n", (rec + 1));
			Sleep(100);
		}
	}
	return 1;
}

unsigned int strLength(char* mas, int startPos)
{
	int i = startPos;
	for (int j = startPos - 1; j >= 0; j--)
	{
		if (mas[j] != '\0') break;
		else i--;
	}
	return i;
}

int crytp_send(int choiceSize, char* buffer, unsigned int& bufSize, int s, char* choice)
{
	if (!CryptEncrypt(sockets[s].DescKey_imp, 0, TRUE, 0, (BYTE*)choice, (DWORD*)&choiceSize, MAX_COMMAND_SIZE))
		printf("ОШИБКА шифрования, код: %x\n", GetLastError());

	if (send(sockets[s].s, choice, choiceSize, 0) < 0)
		return sock_err("send", sockets[s].s);
	if (recv(sockets[s].s, buffer, MAX_BUFFER_SIZE, 0) < 0)
		return sock_err("receive", sockets[s].s);

	bufSize = strLength(buffer, MAX_BUFFER_SIZE);
	if (!CryptDecrypt(sockets[s].DescKey_imp, NULL, TRUE, NULL, (BYTE*)buffer, (DWORD*)&bufSize))
		printf("ОШИБКА дешифрования, код: %x\n", GetLastError());
	return 1;
}

int CryptReal(int s, sockaddr_in addr, const string& ipAddress)
{
	socketExtended result;

	if (!CryptAcquireContext(&result.DescCSP, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0))
	{
		if (!CryptAcquireContext(&result.DescCSP, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))
			printf("ОШИБКА CryptAcquireContext, код: %x\n", GetLastError());
	}

	if (CryptGenKey(result.DescCSP, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &result.DescKey) == 0)
		printf("ОШИБКА CryptGenKey, код: %i\n", GetLastError());

	if (!CryptGetUserKey(result.DescCSP, AT_KEYEXCHANGE, &result.hPublicKey))
		printf("Ошибка CryptGetUserKey (public)\n");
	if (!CryptGetUserKey(result.DescCSP, AT_KEYEXCHANGE, &result.hPrivateKey))
		printf("Ошибка CryptGetUserKey (private)\n");

	char ExpBuf[KEY_BUF_SIZE] = { 0 };
	DWORD len = KEY_BUF_SIZE;

	if (!CryptExportKey(result.hPublicKey, 0, PUBLICKEYBLOB, NULL, (BYTE*)ExpBuf, &len))
		printf("ОШИБКА CryptExportKey, код: %x\n", GetLastError());

	int expBufSize = strLength(ExpBuf, KEY_BUF_SIZE);
	ExpBuf[expBufSize] = expBufSize;

	if (send(s, ExpBuf, (expBufSize + 1), 0) < 0)
		sock_err("send", s);
	char buffer[KEY_BUF_SIZE] = { 0 };
	if (recv(s, buffer, KEY_BUF_SIZE, 0) < 0)
		sock_err("receive", s);

	int bufSize = strLength(buffer, KEY_BUF_SIZE) - 1;
	unsigned int dli = (unsigned char)buffer[bufSize];
	buffer[bufSize] = 0;

	if (!CryptImportKey(result.DescCSP, (BYTE*)buffer, dli, result.hPrivateKey, 0, &result.DescKey_imp))
		printf("ОШИБКА CryptImportKey, код: %x\n", GetLastError());

	result.s = s;
	result.ip_address = ipAddress; 
	sockets.push_back(result);

	return s;
}


void PrepareCommand(int command_num, char* path, char* choice)
{
	memset(choice, 0, MAX_COMMAND_SIZE);
	switch (command_num)
	{
	case 1:  // add server
		choice[0] = 'a';
		choice[1] = '\0';
		break;
	case 2:  // version OS
		choice[0] = 'o';
		choice[1] = '\0';
		break;
	case 3:  // current time
		choice[0] = 't';
		choice[1] = '\0';
		break;
	case 4:  // boot time
		choice[0] = 'm';
		choice[1] = '\0';
		break;
	case 5:  // memory info
		choice[0] = 's';
		choice[1] = '\0';
		break;
	case 6:  // storage info
		choice[0] = 'f';
		choice[1] = '\0';
		break;
	case 7:  // rights
		choice[0] = 'p';
		choice[1] = ' ';
		if (path != NULL && strlen(path) > 0)
			strcpy(&choice[2], path);
		break;
	case 8:  // owner
		choice[0] = 'r';
		choice[1] = ' ';
		if (path != NULL && strlen(path) > 0)
			strcpy(&choice[2], path);
		break;
	case 9:  // close session
		choice[0] = 'e';
		choice[1] = '\0';
		break;
	case 0:  // quit
		choice[0] = 'q';
		choice[1] = '\0';
		break;
	default:  // help
		choice[0] = 'h';
		choice[1] = '\0';
		break;
	}
}

int AddNewServer()
{
	cout << "Введите IP:порт сервера: ";

	string ipAddrAndPort = "";
	cin >> ipAddrAndPort;
	string ipAddress = ipAddrAndPort.substr(0, ipAddrAndPort.find(":"));
	string port = ipAddrAndPort.substr(ipAddrAndPort.find(":") + 1);

	if (port.size() == 0)
		return sock_err("поиск порта", 0);

	int s;
	struct sockaddr_in addr;
	short num_port = (short)atoi(port.c_str());

	WSADATA wsa_data;
	init();

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return sock_err("socket", s);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(num_port);
	addr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	if (connect_200ms(s, addr) != 0)
	{
		s_close(s);
		return sock_err("connect", s);
	}
	cout << "Подключение к серверу установлено!" << endl;

	s = CryptReal(s, addr, ipAddress); // Передаем IP
	cout << "Сервер добавлен под номером: " << sockets.size() << endl;
	servers_amount++;
	return s;
}

string ExtractJsonValue(const char* json, const char* key) {
	string jsonStr(json);
	string keyStr = string("\"") + key + "\":\"";
	size_t keyPos = jsonStr.find(keyStr);
	if (keyPos == string::npos) {
		return jsonStr;
	}

	size_t valueStart = keyPos + keyStr.length();
	size_t valueEnd = jsonStr.find("\"", valueStart);
	if (valueEnd == string::npos) {
		return jsonStr;
	}

	return jsonStr.substr(valueStart, valueEnd - valueStart);
}


string TranslateWellKnownSID(const string& sid) {
	if (sid == "S-1-5-32-544") return "Администраторы (Administrators)";
	if (sid == "S-1-5-32-545") return "Пользователи (Users)";
	if (sid == "S-1-5-32-551") return "Операторы резервного копирования (Backup Operators)";
	if (sid == "S-1-5-32-546") return "Гости (Guests)";
	if (sid == "S-1-5-18") return "Локальная система (Local System)";
	if (sid == "S-1-5-11") return "Аутентифицированные пользователи (Authenticated Users)";
	if (sid == "S-1-1-0") return "Все пользователи (Everyone)";
	if (sid == "S-1-5-32-547") return "Опытные пользователи (Power Users)";
	if (sid == "S-1-5-32-548") return "Операторы учетной записис (Account Operators)";
	if (sid == "S-1-5-32-549") return "Операторы сервера (Server Operators)";
	if (sid == "S-1-5-32-550") return "Операторы печати (Print Operators)";
	return ""; 
}

string FormatAccessRightsBeautiful(const string& raw_data) {
	stringstream result;
	stringstream ss(raw_data);
	string line;

	result << "=== ПРАВА ДОСТУПА ===" << endl << endl;

	int entry_count = 0;
	while (getline(ss, line)) {
		if (line.empty()) continue;

		stringstream line_ss(line);
		string name, sid, mask_bits;
		string ace_list;

		line_ss >> name >> sid >> mask_bits;

		if (line_ss) {
			getline(line_ss, ace_list);
		}

		if (name.empty() || sid.empty() || mask_bits.empty()) continue;

		bool valid_mask = true;
		for (char c : mask_bits) {
			if (c != '0' && c != '1') {
				valid_mask = false;
				break;
			}
		}
		if (!valid_mask) continue;

		entry_count++;
		result << "ЗАПИСЬ " << entry_count << ":" << endl;

		// Используем переведенное имя SID
		string display_name = TranslateWellKnownSID(sid);
		if (display_name.empty()) {
			// Если не нашли перевод, используем оригинальное имя, но фильтруем вопросы
			if (name.find("???") == string::npos) {
				display_name = name;
			}
			else {
				display_name = "Неизвестный субъект";
			}
		}

		result << "Субъект: " << display_name << endl;
		result << "SID: " << sid << endl;
		result << "Маска доступа: " << mask_bits << " (бинарная)" << endl;

		// Разбираем ACE
		vector<string> allowed;
		vector<string> denied;

		if (!ace_list.empty()) {
			stringstream ace_ss(ace_list);
			string ace_item;

			while (getline(ace_ss, ace_item, ';')) {
				if (ace_item.empty()) continue;				
				string clean_ace = ace_item;
				clean_ace.erase(0, clean_ace.find_first_not_of(" \t"));

				size_t allowed_pos = clean_ace.find(" --- ALLOWED");
				size_t denied_pos = clean_ace.find(" --- DENIED");

				if (allowed_pos != string::npos) {
					clean_ace = clean_ace.substr(0, allowed_pos);
					if (!clean_ace.empty()) {
						allowed.push_back(clean_ace);
					}
				}
				else if (denied_pos != string::npos) {
					clean_ace = clean_ace.substr(0, denied_pos);
					if (!clean_ace.empty()) {
						denied.push_back(clean_ace);
					}
				}
			}
		}
		if (!allowed.empty()) {
			result << endl << "РАЗРЕШЕННЫЕ ПРАВА:" << endl;
			for (const auto& right : allowed) {
				result << "  • " << right << endl;
			}
		}
		if (!denied.empty()) {
			result << endl << "ЗАПРЕЩЕННЫЕ ПРАВА:" << endl;
			for (const auto& right : denied) {
				result << "  • " << right << endl;
			}
		}

		result << endl;
	}

	if (entry_count == 0) {
		result << "Информация о правах доступа не найдена." << endl;
	}

	return result.str();
}

int io_serv() {
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	char choice[MAX_COMMAND_SIZE];
	char path[MAX_COMMAND_SIZE] = { 0 };

	unsigned int choiceSize;
	unsigned int bufSize;

	int selected_server = 0;
	int command_num = -1;

	ShowMenu();

	do {
		memset(buffer, 0, MAX_BUFFER_SIZE);
		memset(choice, 0, MAX_COMMAND_SIZE);
		memset(path, 0, MAX_COMMAND_SIZE);

		ShowServers();
		cout << "\nДля работы с подключенным сервером введите его номер";
		cout << "\nИли выберите команду:\n0 - добавить сервер, \n-1 - справка, \n-2 - выход\n";
		scanf("%d", &selected_server);

		if (selected_server == 0) {
			AddNewServer();
			continue;
		}

		if (selected_server == -1) {
			ShowMenu();
			continue;
		}

		if (selected_server == -2) {
			goto END;
		}

		if (selected_server > 0) {
			int server_index = selected_server - 1;
			if (server_index >= (int)sockets.size()) {
				cout << "Ошибка: сервер с номером " << selected_server << " не существует!" << endl;
				continue;
			}
			cout << "Введите номер команды: ";
			scanf("%d", &command_num);
			if (command_num == 7 || command_num == 8) {
				cout << "Введите путь: ";
				char temp;
				scanf("%c", &temp);
				scanf("%[^\n]", path);
			}
			PrepareCommand(command_num, path, choice);
			choiceSize = strlen(choice);
			switch (choice[0])
			{
			case 'o':  // Версия ОС
			{
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "version");
				cout << endl << "=== ВЕРСИЯ ОС ===" << endl << value << endl;
				break;
			}
			case 't':  // Текущее время
			{
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "time");
				cout << endl << "=== ТЕКУЩЕЕ ВРЕМЯ ===" << endl << value << endl;
				break;
			}
			case 'm':  // Время работы системы
			{
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "boot_time");
				cout << endl << "=== ВРЕМЯ РАБОТЫ СИСТЕМЫ ===" << endl << value << endl;
				break;
			}
			case 's':  // Информация о памяти
			{
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "memory_info");
				cout << endl << "=== ИНФОРМАЦИЯ О ПАМЯТИ ===" << endl << value << endl;
				break;
			}
			case 'f':  // Информация о дисках
			{
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "storage_info");
				cout << endl << "=== ИНФОРМАЦИЯ О ДИСКАХ ===" << endl << value << endl;
				break;
			}
			case 'p':  // Права доступа
			{
				if (choiceSize < MIN_PATH_SIZE)
				{
					cout << "Неверный путь" << endl;
					break;
				}
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "access_rights");
				string formatted = FormatAccessRightsBeautiful(value);
				cout << formatted << endl;
				break;
			}
			case 'r':  // Владелец
			{
				if (choiceSize < MIN_PATH_SIZE)
				{
					cout << "Неверный путь" << endl;
					break;
				}
				if (crytp_send(choiceSize, buffer, bufSize, server_index, choice) == -1)
					return -1;
				string value = ExtractJsonValue(buffer, "owner");
				cout << endl << "=== ВЛАДЕЛЕЦ ===" << endl << value << endl;
				break;
			}
			case 'a':  // Добавить сервер
			{
				AddNewServer();
				break;
			}
			case 'e':  // Закрыть сессию
			{
				if (!CryptEncrypt(sockets[server_index].DescKey_imp, 0, TRUE, 0, (BYTE*)choice, (DWORD*)&choiceSize, MAX_COMMAND_SIZE))
					printf("ОШИБКА, %x", GetLastError());

				if (send(sockets[server_index].s, choice, strlen(choice), 0) < 0)
					return sock_err("send", sockets[server_index].s);

				if (!CryptDecrypt(sockets[server_index].DescKey_imp, NULL, TRUE, NULL, (BYTE*)choice, (DWORD*)&choiceSize))
					printf("ОШИБКА, %x", GetLastError());

				cout << "Сессия для сервера " << selected_server << " закрыта" << endl;
				RemoveClosedServer(server_index);
				break;
			}
			case 'q':  // Выход
			{
				crytp_send(choiceSize, buffer, bufSize, server_index, choice);
				string value = ExtractJsonValue(buffer, "");
				if (!value.empty()) {
					cout << endl << value << endl;
				}
				goto END;
			}
			case 'h':  // Справка
			{
				ShowMenu();
				continue;
			}
			default:
			{
				printf("Неверная команда!\n");
				continue;
			}
			}
		}

	} while (true);

END:
	cout << "Соединение закрыто" << endl;
	for (size_t i = 0; i < sockets.size(); i++) {
		closesocket(sockets[i].s);
	}
	WSACleanup();
	return 0;
}

int main() {
	setlocale(LC_ALL, "Russian");
	return io_serv();
}