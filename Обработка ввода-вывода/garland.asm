; Определения регистров
.def ZERO = R18       ; Нулевое значение 
.def PIN_D = R19      ; Хранит значение PIND для чтения состояния порта D
.def TMP = R20          ; Временный регистр для общих операций
.def MODE = R21       ; Текущий режим работы (1, 2 или 3)
.def STATE1 = R23     ; Первое состояние гирлянды (0xFF, 0xAA или y)
.def STATE2 = R24     ; Второе состояние гирлянды (0x00, 0x55 или -y)
.def YVAL = R25       ; Хранит значение y 


; Векторы прерываний
.org $000             
    JMP reset
.org INT0addr         
    JMP INT0_HANDLER
.org INT1addr         
    JMP INT1_HANDLER

; Инициализация микроконтроллера
RESET:
    ; Инициализация регистров
    LDI ZERO, 0x00    ; Устанавливаем нулевое значение
	LDI TMP, 0x01       ; Начальное значение для R0
	LDI R19, 0x03    
    MOV R0, TMP         
    CLR TMP             
    MOV R1, TMP         ; Обнуляем R1-R3
    MOV R2, TMP
    MOV R3, TMP

    ; Настройка портов
    LDI MODE, 1       ; Начальный режим = 1
    SER TMP           ; Устанавливаем все биты в 1 (0xFF)
    OUT DDRA, TMP     ; PORTA как выход (светодиоды)
    OUT DDRB, TMP     ; PORTB как выход (светодиоды)
    CLR TMP
    OUT DDRC, TMP     ; PORTC как вход (для ввода y)
    LDI R16, 0x0F     ; Адрес EEPROM (0x0F)
    LDI R17, 0x0F
	LDI TMP, 0b00000011
    OUT DDRD, TMP     ; PD0-PD1 как выходы (для номера режима), остальные — входы

    ; Настройка стека
    LDI TMP,HIGH(RAMEND) 
    OUT SPH, TMP
    LDI TMP, LOW(RAMEND)  
    OUT SPL, TMP

    ; Настройка прерываний
    LDI TMP, 0x0F       ; INT0 и INT1 срабатывают по нарастающему фронту
    OUT MCUCR, TMP
    LDI TMP, 0xC0       ; Разрешаем INT0 и INT1
    OUT GICR, TMP
    OUT GIFR, TMP       ; Сбрасываем флаги прерываний

    ; Начальные значения
    LDI YVAL, 0x71      ; Начальное значение y = 0x71
    SEI                 ; Разрешаем глобальные прерывания
    CALL load_mode      ; Читаем сохраненный режим из EEPROM
    CALL update_mode    ; Применяем настройки режима
    CALL run_lights     ; Запускаем основной цикл

; Основные функции программы

; Задержка ~500 мс (частота 2 Гц)
wait:
    LDI R31, 20
    LDI R30, 20
    LDI R29, 20
wait_loop:
    DEC R29
    BRNE wait_loop
    DEC R30
    BRNE wait_loop
    DEC R31
    BRNE wait_loop
    NOP
    NOP
    RET

; Основной цикл работы гирлянды
run_lights:
    OUT PORTD, MODE     ; Выводим номер режима на PD0-PD1
    CPI MODE, 3         ; Если режим 3, проверяем ввод y
    BREQ check_input
blink_loop:
    OUT PORTB, STATE1   ; Первое состояние на PORTB
    OUT PORTA, STATE2   ; Второе состояние на PORTA
    CALL wait           ; Ждем 500 мс
    OUT PORTB, STATE2   ; Меняем состояния местами
    OUT PORTA, STATE1
    CALL wait           ; Ждем 500 мс
    RJMP run_lights     ; Повторяем цикл

; Проверка нажатия PD7 в режиме 3
check_input:
    IN PIN_D, PIND      ; Читаем состояние порта D
    CPI PIN_D, 131      ; Проверяем, нажата ли PD7 
    BRNE blink_loop     ; Если не нажата, продолжаем мигание
    CALL wait           ; Антидребезг: ждем 500 мс
    IN PIN_D, PIND      ; Перепроверяем
    CPI PIN_D, 131
    BREQ get_y          ; Если PD7 все еще нажата, переходим к вводу y
    RJMP blink_loop

; Ввод числа y через PORTC
get_y:
    CLI                 ; Запрещаем прерывания на время ввода
    LDI YVAL, 0x00
    OUT PORTA, YVAL     ; Гасим светодиоды
    OUT PORTB, YVAL
    CALL wait           ; Ждем 500 мс
    LDI ZERO, 0
    IN YVAL, PINC       ; Читаем значение с PORTC
    CP YVAL, ZERO       ; Если ничего не нажато, повторяем
    BREQ get_y
    MOV STATE1, YVAL    ; Сохраняем y
    MOV TMP, YVAL       ; Копируем y во временный регистр
    LDI R16, 0x80       ; Маска для старшего бита
    EOR TMP, R16        ; Инвертируем старший бит для -y (прямой код) в TMP
    MOV STATE2, TMP     ; Сохраняем -y
wait_release:
    LDI R27, 100        ; Счетчик для таймаута
check_timeout:
    IN YVAL, PINC
    CP YVAL, ZERO       ; Если кнопки отпущены, выходим
    BREQ end_input
    DEC R27
    BRNE check_timeout  ; Если таймаут, все равно выходим
end_input:
    IN PIN_D, PIND      ; Читаем PIND в PIN_D, а не в YVAL
    CPI PIN_D, 131      ; Проверяем, отпущена ли PD7
    BRNE get_y
    SEI                 ; Разрешаем прерывания
    RJMP blink_loop

; Настройки режимов
set_mode1:
    LDI STATE1, 0xFF    ; Первое состояние (0xFF)
    LDI STATE2, 0x00    ; Второе состояние (0x00)
    RET

set_mode2:
    LDI STATE1, 0xAA    ; Первое состояние (0xAA)
    LDI STATE2, 0x55    ; Второе состояние (0x55)
    RET

set_mode3:
    MOV STATE1, YVAL    ; Первое состояние = y
    MOV TMP, YVAL       ; Копируем y во временный регистр
    LDI R16, 0x80       ; Маска для старшего бита
    EOR TMP, R16        ; Инвертируем старший бит для -y (прямой код) в TMP
    MOV STATE2, TMP     ; Сохраняем -y в STATE2
    RET

; Работа с EEPROM
save_mode:
    SBIC EECR, EEWE     ; Проверяем, не идет ли уже запись
    RJMP save_mode
    OUT EEARH, ZERO     ; Старший байт адреса (0x00)
    OUT EEARL, R17      ; Младший байт адреса (0x0F)
    OUT EEDR, MODE      ; Записываем MODE в регистр данных
    SBI EECR, EEMWE     ; Разрешаем запись
    SBI EECR, EEWE      ; Запускаем запись
    RET

load_mode:
    SBIC EECR, EEWE     ; Ждем окончания предыдущей записи
    RJMP load_mode
    OUT EEARH, ZERO     ; Адрес EEPROM (0x0F)
    OUT EEARL, R17
    SBI EECR, EERE      ; Запускаем чтение
    IN MODE, EEDR       ; Загружаем данные в MODE
    RET

; Смена режима и сохранение в EEPROM
update_mode:
    CALL save_mode      ; Сохраняем новый режим
    CPI MODE, 1         ; Проверяем режим и вызываем
    BREQ set_mode1      ; соответствующую функцию
    CPI MODE, 2
    BREQ set_mode2
    CPI MODE, 3
    BREQ set_mode3
    RET

; Обработчики прерываний

; Обработчик прерывания INT0 (увеличение режима)
INT0_HANDLER:
    IN TMP, SREG        ; Сохраняем флаги в R28
    CALL load_mode      ; Читаем текущий режим из EEPROM
    INC MODE            ; Увеличиваем режим на 1
    CPI MODE, 4         ; Если режим > 3, сбрасываем до 1
    BREQ reset_to_1
    CALL update_mode    ; Применяем новый режим
    OUT SREG, TMP      ; Восстанавливаем флаги
    RETI                ; Выходим из прерывания

; Обработчик прерывания INT1 (уменьшение режима)
INT1_HANDLER:
    IN TMP, SREG        ; Сохраняем флаги
    CALL load_mode
    DEC MODE            ; Уменьшаем режим
    CPI MODE, 0         ; Если режим < 1, устанавливаем 3
    BREQ reset_to_3
    CALL update_mode
    OUT SREG, TMP       ; Восстанавливаем флаги
    RETI

; Обработка переполнения режима (сверху)
reset_to_1:
    LDI MODE, 1         ; Сбрасываем режим до 1
    CALL update_mode
    OUT SREG, R28       ; Восстанавливаем флаги
    RETI

; Обработка переполнения режима (снизу)
reset_to_3:
    LDI MODE, 3         ; Устанавливаем режим 3
    CALL update_mode
    OUT SREG, R28
    RETI
