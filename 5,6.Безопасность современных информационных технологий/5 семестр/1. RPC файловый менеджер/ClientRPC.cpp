#pragma comment(lib, "rpcrt4.lib")
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "InterfaceRPC.h"

using namespace std;

void* __RPC_USER MIDL_user_allocate(size_t size) { return malloc(size); }
void __RPC_USER MIDL_user_free(void* p) { free(p); }

int Rpc_OpenSession(handle_t hBinding, const string& login, const string& password, int* err)
{
    RpcTryExcept{
        return OpenSession(hBinding, (unsigned char*)login.c_str(), (unsigned char*)password.c_str(), err);
    }
        RpcExcept(1) {
        *err = RpcExceptionCode();
        return -1;
    }
    RpcEndExcept
}

int Rpc_CloseSession(handle_t hBinding, int* err)
{
    RpcTryExcept{
        return CloseSession(hBinding, err);
    }
        RpcExcept(1) {
        *err = RpcExceptionCode();
        return -1;
    }
    RpcEndExcept
}

int Rpc_CopyToServer(handle_t hBinding, const string& path, const string& local, int* err)
{
    ifstream file(local, ios::binary | ios::ate);
    if (!file.is_open()) {
        std::cout << "[CLIENT] Cannot open local file" << endl;
        return -1;
    }
    long size = (long)file.tellg();
    file.seekg(0, ios::beg);
    unsigned char* buf = new unsigned char[size];
    file.read((char*)buf, size);
    file.close();
    int res = CopyToServer(hBinding, (unsigned char*)path.c_str(), size, buf, err);
    delete[] buf;
    return res;
}

int Rpc_DownloadFromServer(handle_t hBinding, const string& remote, const string& local, int* err)
{
    *err = 0;
    long size = FileSize(hBinding, (unsigned char*)remote.c_str());
    if (size == 0) {
        *err = ERROR_FILE_NOT_FOUND;
        return -1;
    }
    unsigned char* Arr = new unsigned char[size];
    memset(Arr, 0, size);
    if (DownloadFromServer(hBinding, (unsigned char*)remote.c_str(), size, Arr, err) != 0) {
        cout << "[CLIENT] DownloadFromServer failed" << endl;
        delete[] Arr;
        return -1;
    }
    ofstream ofs(local, ios::binary);
    if (!ofs.is_open()) {
        *err = GetLastError();
        cout << "[CLIENT] Cannot open local file: " << local << endl;
        delete[] Arr;
        return -1;
    }
    ofs.write((char*)Arr, size);
    ofs.close();
    delete[] Arr;
    cout << "[CLIENT] Downloaded " << size << " bytes to " << local << endl;
    return 0;
}

int Rpc_DeleteOnServer(handle_t hBinding, const string& path, int* err)
{
    RpcTryExcept{
        return DeleteOnServer(hBinding, (unsigned char*)path.c_str(), err);
    }
        RpcExcept(1) {
        *err = RpcExceptionCode();
        return -1;
    }
    RpcEndExcept
}


int main()
{
    RPC_STATUS status;
    RPC_CSTR bindingStr = NULL;
    RPC_BINDING_HANDLE hBinding = NULL;
    string ip = "192.168.0.189";
    status = RpcStringBindingComposeA(NULL, (RPC_CSTR)"ncacn_ip_tcp",
        (RPC_CSTR)ip.c_str(), (RPC_CSTR)"1500", NULL, &bindingStr);
    if (status != RPC_S_OK) {
        cout << "RpcStringBindingComposeA failed: " << status << endl;
        return 1;
    }
    status = RpcBindingFromStringBindingA(bindingStr, &hBinding);
    RpcStringFreeA(&bindingStr);
    if (status != RPC_S_OK) {
        cout << "RpcBindingFromStringBindingA failed: " << status << endl;
        return 1;
    }
    cout << "[CLIENT] Connected to server" << endl;
    int err = 0;
    cout << "Login: ";
    string login; getline(cin, login);
    cout << "Password: ";
    string pass; getline(cin, pass);
    if (Rpc_OpenSession(hBinding, login, pass, &err) != 0) {
        cout << "[CLIENT] Authentication failed, code: " << err << endl;
        RpcBindingFree(&hBinding);
        return 1;
    }
    cout << "[CLIENT] Auth success!" << endl;
    bool run = true;
    while (run)
    {
        cout << "\nMenu:\n1) Upload file\n2) Download file\n3) Delete file\n4) Close session & exit\nChoice: ";
        string choice; getline(cin, choice);
        int option = atoi(choice.c_str());
        switch (option)
        {
        case 1: {
            string local, remote;
            cout << "Local file path: "; getline(cin, local);
            cout << "Server target path: "; getline(cin, remote);
            if (Rpc_CopyToServer(hBinding, remote, local, &err) == 0)
                cout << "Upload OK" << endl;
            else {
                if (err == 13)
                    cout << "Upload failed : access denied" << endl;
                else
                    cout << "Upload failed, code=" << err << endl;
            }
            break;
        }

        case 2: {
            string remote, local;
            cout << "Server file path: "; getline(cin, remote);
            cout << "Save as (local path): "; getline(cin, local);
            if (Rpc_DownloadFromServer(hBinding, remote, local, &err) == 0)
                cout << "Download OK" << endl;
            else {
                if (err == 2 || err == 13)
                    cout << "Download failed: access denied" << endl;
                else
                    cout << "Download failed, code=" << err << endl;
            }
            break;
        }

        case 3: {
            string remote;
            cout << "Server file to delete: "; getline(cin, remote);
            if (Rpc_DeleteOnServer(hBinding, remote, &err) == 0)
                cout << "Delete OK" << endl;
            else {
                if (err == 13)
                    cout << "Delete failed: access denied" << endl;
                else
                    cout << "Delete failed, code=" << err << endl;
            }
            break;
        }

        case 4:
            Rpc_CloseSession(hBinding, &err);
            cout << "[CLIENT] Session closed. Exiting..." << endl;
            run = false;
            break;
        default:
            cout << "Invalid choice" << endl;
        }
    }
    RpcBindingFree(&hBinding);
    return 0;
}
