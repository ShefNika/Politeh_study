use32

start:
    pushad ; 
    push ebp ; 
    mov ebp, esp
    sub esp, 24h        ; выделить 36 байт для локальных переменных [ebp-4]=база kernel32,  [ebp-8]=имя функции,
                        ; [ebp-12]=Ordinal,        [ebp-16]=NamePtr,
                        ; [ebp-20]=AddrTable,      [ebp-24]=LoadLibraryA,
                        ; [ebp-28]=база user32,    [ebp-32]=MessageBoxA
                        ; [ebp-36]=ExitProcess     

    ; Найти kernel32.dll через PEB 
    xor esi, esi
    mov ebx, [fs:30h + esi] ; загружаем в ebx адрес PEB
    mov ebx, [ebx + 0x0C] ; PEB_LDR_DATA
    mov ebx, [ebx + 0x14] ; InMemoryOrderModuleList (двусвязный список всех dll)
    mov ebx, [ebx] ; второй эл-т списка ntdll
    mov ebx, [ebx] ; третий kernel32
    mov ebx, [ebx + 0x10] ; ImageBase kernel32
    mov [ebp-4], ebx ; сохр базу kernel32

    ; Найти LoadLibraryA в kernel32 
    xor esi, esi
    push esi
    push 0x41797261             ; "aryA"
    push 0x7262694C             ; "Libr"
    push 0x64616F4C             ; "Load"
    mov [ebp-8], esp

    mov eax, [ebx + 3Ch] ; смещение PE-заголовка 
    add eax, ebx ; абсолютный адрес PE-заголовка
    mov eax, [eax + 78h] ; смещение Export Directory (из Optional Header)
    add eax, ebx ; абсолютный адрес Export Directory

    mov ecx, [eax + 24h] ; ECX = смещение таблицы Ordinal'ов
    add ecx, ebx
    mov [ebp-12], ecx

    mov edi, [eax + 20h] ; EDI = смещение таблицы имён
    add edi, ebx
    mov [ebp-16], edi

    mov edx, [eax + 1Ch] ; EDX = смещение таблицы адресов
    add edx, ebx
    mov [ebp-20], edx

    mov edx, [eax + 14h] ; EDX = количество функций (счётчик цикла)
    xor eax, eax

    .loop1:
        mov edi, [ebp-16] ; EDI = таблица имён
        mov esi, [ebp-8] ; ESI = "LoadLibraryA\0"
        xor ecx, ecx
        mov cl, 0x0E ; ECX = 14
        dec ecx ; ECX = 13 (длина "LoadLibraryA\0")
        cld ; флаг направления = вперёд (для cmpsb)
        mov edi, [edi + eax*4] ; EDI = смещение имени функции 
        add edi, ebx
        repe cmpsb ; сравниваем ECX байт: [ESI] vs [EDI]
        jz start.found1
        inc eax
        cmp eax, edx
        jb start.loop1

    .found1:
        mov ecx, [ebp-12] ; ECX = таблица Ordinal'ов
        mov edx, [ebp-20]  ; EDX = таблица адресов
        movzx eax, word [ecx + eax*2] ; EAX = ordinal функции (2 байта)
        mov eax, [edx + eax*4] ; EAX = относительный адреса функции
        add eax, ebx
        mov [ebp-24], eax

    ; Найти ExitProcess в kernel32 (ebx всё ещё = kernel32) 
    push 0x58737365             ; "essX" ('X' чтоб дозаполнить, сравниваем только 11 байт)
    push 0x636F7250             ; "Proc"
    push 0x74697845             ; "Exit"
    mov [ebp-8], esp            ; указатель на "ExitProcess..."

    mov eax, [ebx + 3Ch]
    add eax, ebx
    mov eax, [eax + 78h]
    add eax, ebx

    mov ecx, [eax + 24h]
    add ecx, ebx
    mov [ebp-12], ecx

    mov edi, [eax + 20h]
    add edi, ebx
    mov [ebp-16], edi

    mov edx, [eax + 1Ch]
    add edx, ebx
    mov [ebp-20], edx

    mov edx, [eax + 14h]
    xor eax, eax

    .loop_exit:
        mov edi, [ebp-16]
        mov esi, [ebp-8]
        xor ecx, ecx
        cld
        mov edi, [edi + eax*4]
        add edi, ebx
        add cx, 0x0B            ; len("ExitProcess") = 11
        repe cmpsb
        jz start.found_exit
        inc eax
        cmp eax, edx
        jb start.loop_exit

    .found_exit:
        mov ecx, [ebp-12]
        mov edx, [ebp-20]
        movzx eax, word [ecx + eax*2]
        mov eax, [edx + eax*4]
        add eax, ebx
        mov [ebp-36], eax       ; сохранили адрес ExitProcess

    add esp, 0Ch                ; убрали "ExitProcess..." (3 DWORD = 12 байт)
    add esp, 10h                ; убрали "LoadLibraryA\0" (4 DWORD = 16 байт)

    ; LoadLibraryA("user32.dll") 
    xor eax, eax
    mov al, 0x6C      ; 'l'
    shl eax, 8        ; сдвиг: 0x0000006C → 0x00006C00
    mov al, 0x6C      ; 'l' → EAX = 0x00006C6C = "ll\0\0"
    push eax           ; кладём "ll\0\0"
    push 0x642E3233    ; "23.d"
    push 0x72657375    ; "user"
    push esp           ; аргумент: указатель на "user32.dll\0"
    call dword [ebp-24] ; LoadLibraryA("user32.dll")
    add esp, 0Ch       ; убрать "user32.dll" (но не указатель — call уже вернул)
    mov [ebp-28], eax  ; EAX = база user32.dll
    mov ebx, eax       ; EBX тоже = база user32 (для дальнейшего парсинга)

    ; Найти MessageBoxA в user32.dll 
    xor eax, eax
    mov al, 0x41
    shl eax, 8
    mov al, 0x78
    shl eax, 8
    mov al, 0x6F
    push eax
    push 0x42656761
    push 0x7373654D
    mov [ebp-8], esp

    mov eax, [ebx + 3Ch]
    add eax, ebx
    mov eax, [eax + 78h]
    add eax, ebx

    mov ecx, [eax + 24h]
    add ecx, ebx
    mov [ebp-12], ecx

    mov edi, [eax + 20h]
    add edi, ebx
    mov [ebp-16], edi

    mov edx, [eax + 1Ch]
    add edx, ebx
    mov [ebp-20], edx

    mov edx, [eax + 14h]
    xor eax, eax

    .loop2:
        mov edi, [ebp-16]
        mov esi, [ebp-8]
        xor ecx, ecx
        cld
        mov edi, [edi + eax*4]
        add edi, ebx
        add cx, 0Ch
        repe cmpsb
        jz start.found2
        inc eax
        cmp eax, edx
        jb start.loop2

    .found2:
        mov ecx, [ebp-12]
        mov edx, [ebp-20]
        movzx eax, word [ecx + eax*2]
        mov eax, [edx + eax*4]
        add eax, ebx
        mov [ebp-32], eax

    add esp, 0Ch

    ; MessageBoxA(NULL, "Hack", "Error", MB_OK) 
    ; Строим заголовок "Error\0" на стеке 
    xor eax, eax
    mov al, 0x72                ; последний 'r'
    push eax                    ; "r\0\0\0"
    push 0x6F727245             ; "Erro" → в памяти: "Error\0\0\0"
    mov ecx, esp                ; ecx → "Error"

    ; Строим текст "Hack\0" на стеке (H=48 a=61 c=63 k=6B)
    xor eax, eax
    push eax                    ; "\0\0\0\0"
    push 0x6B636148             ; "Hack" → в памяти: "Hack\0\0\0\0"
    mov edx, esp                ; edx → "Hack"

    xor esi, esi
    push esi                    ; uType = MB_OK = 0
    push ecx                    ; lpCaption = "Error"
    push edx                    ; lpText    = "Hack"
    push esi                    ; hWnd = NULL
    call dword [ebp-32]         ; MessageBoxA → после OK возвращается сюда

    ;  Завершить процесс после закрытия окна 
    xor eax, eax
    push eax                    ; dwExitCode = 0
    call dword [ebp-36]         ; ExitProcess(0) 