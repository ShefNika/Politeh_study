; Определение констант для семисегментных индикаторов
.equ null        = 0b00000000 
.equ zero        = 0b00111111
.equ one         = 0b00000110
.equ two         = 0b01011011
.equ three       = 0b01001111
.equ four        = 0b01100110
.equ five        = 0b01101101
.equ six         = 0b01111101
.equ seven       = 0b00000111
.equ eight       = 0b01111111
.equ nine        = 0b01101111

; Определение адресов в SRAM
.equ CurrentDigitDisplay = 0x0064 ; Текущий индикатор для мультиплексирования
.equ BlinkCounter       = 0x0065 ; Счетчик мигания
.equ LockState          = 0x0066 ; Состояние блокировки 
.equ LockCounter        = 0x0067 ; Счетчик задержки
.equ ButtonHoldCounterPD0 = 0x0069  ; Счётчик времени удержания для PD0 
.equ ButtonHoldCounterPD1 = 0x006A  ; Счётчик времени удержания для PD1 
.equ HoldStagePD0       = 0x006B    ; Флаг этапа удержания для PD0 (0 - первый этап, 1 - второй этап)
.equ HoldStagePD1       = 0x006C ; ; Флаг этапа удержания для PD1 (0 - первый этап, 1 - второй этап)
.equ AutoIncrementActive = 0x006D  ; Флаг активности автоинкремента (PD0)
.equ AutoDecrementActive = 0x006E  ; Флаг активности автодекремента (PD1)

; Определение регистров
.def CurrentDisplay = R18 ; Текущий разряд
.def Digit0         = R19
.def Digit1         = R20
.def Digit2         = R21
.def Digit3         = R22
.def PinD_          = R23
.def Button0Pressed = R24 
.def Button1Pressed = R25
.def Blink          = R26
.def PinDigit0      = R27
.def PinDigit1      = R28
.def PinDigit2      = R29
.def PinDigit3      = R30
.def IncorrectAttempts = R31

; Векторная таблица прерываний
.org $000
    JMP RESET
.org $002
    JMP INT0_HANDLER
.org $004
    JMP INT1_HANDLER
.org $01A
    JMP TIMER1_COMPA

; Начало программы
.org $02A
RESET:
    ; Инициализация стека
    LDI R16, HIGH(RAMEND)
    OUT SPH, R16
    LDI R16, LOW(RAMEND)
    OUT SPL, R16

    ; Инициализация портов
    SER R16
    OUT DDRC, R16
    CLR R16
    OUT DDRD, R16
    LDI R16, 0xFF
    OUT PORTD, R16
    CLR R16
    OUT PORTA, R16
    OUT PORTC, R16
    LDI R16, 0xCF ; 0b11001111
    OUT DDRA, R16 ; PA0-PA3 — выходы (выбор разряда индикаторов), PA6-PA7 — выходы (светодиоды)

    ; Инициализация начального состояния
    CLR CurrentDisplay
    CLR Digit0
    CLR Digit1
    CLR Digit2
    CLR Digit3
    CLR Button0Pressed
    CLR Button1Pressed
    LDI Blink, 0b00000001
    CLR PinDigit0
    CLR PinDigit1
    CLR PinDigit2
    CLR PinDigit3
    CLR IncorrectAttempts

    ; Считывание PIN-кода из EEPROM
    LDI R16, 0x00
    OUT EEARH, R16 ; EEARH - старший байт адреса EEPROM (только 2 младших бита используются)
    LDI R16, 0x00
    OUT EEARL, R16 ; EEARL - младший байт адреса EEPROM
    SBI EECR, EERE ; EECR - управляет операциями, EERE - бит чтения в EECR
    IN PinDigit0, EEDR ; EEDR - содержит прочитанные данные

    LDI R16, 0x01
    OUT EEARL, R16
    SBI EECR, EERE
    IN PinDigit1, EEDR

    LDI R16, 0x02
    OUT EEARL, R16
    SBI EECR, EERE
    IN PinDigit2, EEDR

    LDI R16, 0x03
    OUT EEARL, R16
    SBI EECR, EERE
    IN PinDigit3, EEDR

    ; Инициализация переменных в SRAM
    CLR R16
    STS CurrentDigitDisplay, R16
    STS BlinkCounter, R16
    STS LockState, R16
    STS LockCounter, R16
    STS LockCounter+1, R16
	STS ButtonHoldCounterPD0, R16
    STS ButtonHoldCounterPD1, R16
    STS HoldStagePD0, R16
    STS HoldStagePD1, R16
	STS AutoIncrementActive, R16
    STS AutoDecrementActive, R16
	

    ; Настройка Timer1 (5 мс, 200 Гц)
    CLR R16
    OUT TCCR1A, R16
    OUT TCCR1B, R16
    LDI R16, HIGH(156)
    OUT OCR1AH, R16
    LDI R16, LOW(156)
    OUT OCR1AL, R16
    LDI R16, 0b00001100 ; Режим CTC, предделитель 256
    OUT TCCR1B, R16
    LDI R16, 0b00001000  ; Разрешение прерывания по совпадению OCR1A Timer1
    OUT TIMSK, R16
    ;LDI R16, 0b01000000 ; Сброс флага прерывания
    ;OUT TIFR, R16

    ; Настройка прерываний INT0 и INT1
    LDI R16, 0x0A ; 0b00001010
    OUT MCUCR, R16 ; INT0 и INT1 по спаду фронта
    LDI R16, 0xC0 ; 0b11000000
    OUT GICR, R16 ; Разрешить INT0 и INT1
    LDI R16, 0xC0
    OUT GIFR, R16 ; Сбросить флаги INTF0 и INTF1

    SEI ; Глобальное разрешение прерываний

; Основной цикл
main_loop:
    RJMP main_loop

; Подпрограмма преобразования значения цифры
show_digit:
    CPI R16, 0
    BREQ digit_zero
    CPI R16, 1
    BREQ digit_one
    CPI R16, 2
    BREQ digit_two
    CPI R16, 3
    BREQ digit_three
    CPI R16, 4
    BREQ digit_four 
    CPI R16, 5
    BREQ digit_five
    CPI R16, 6
    BREQ digit_six
    CPI R16, 7
    BREQ digit_seven
    CPI R16, 8
    BREQ digit_eight
    CPI R16, 9
    BREQ digit_nine
    RET

digit_zero:  LDI R16, zero    RET
digit_one:   LDI R16, one     RET
digit_two:   LDI R16, two     RET
digit_three: LDI R16, three   RET
digit_four:  LDI R16, four    RET
digit_five:  LDI R16, five    RET
digit_six:   LDI R16, six     RET
digit_seven: LDI R16, seven   RET
digit_eight: LDI R16, eight   RET
digit_nine:  LDI R16, nine    RET

; Подпрограммы инкремента и декремента
increment_digit:
    CPI CurrentDisplay, 0
    BREQ inc_digit0
    CPI CurrentDisplay, 1
    BREQ inc_digit1
    CPI CurrentDisplay, 2
    BREQ inc_digit2
    CPI CurrentDisplay, 3
    BREQ inc_digit3
    RET
inc_digit0: INC Digit0    CPI Digit0, 10    BRNE exit_increment    CLR Digit0    RJMP exit_increment
inc_digit1: INC Digit1    CPI Digit1, 10    BRNE exit_increment    CLR Digit1    RJMP exit_increment
inc_digit2: INC Digit2    CPI Digit2, 10    BRNE exit_increment    CLR Digit2    RJMP exit_increment
inc_digit3: INC Digit3    CPI Digit3, 10    BRNE exit_increment    CLR Digit3
exit_increment:
    RET

decrement_digit:
    CPI CurrentDisplay, 0
    BREQ dec_digit0
    CPI CurrentDisplay, 1
    BREQ dec_digit1
    CPI CurrentDisplay, 2
    BREQ dec_digit2
    CPI CurrentDisplay, 3
    BREQ dec_digit3
    RET
dec_digit0: DEC Digit0    CPI Digit0, 0xFF    BRNE exit_decrement    LDI Digit0, 9    RJMP exit_decrement
dec_digit1: DEC Digit1    CPI Digit1, 0xFF    BRNE exit_decrement    LDI Digit1, 9    RJMP exit_decrement
dec_digit2: DEC Digit2    CPI Digit2, 0xFF    BRNE exit_decrement    LDI Digit2, 9    RJMP exit_decrement
dec_digit3: DEC Digit3    CPI Digit3, 0xFF    BRNE exit_decrement    LDI Digit3, 9
exit_decrement:
    RET

; Обработчик INT0
INT0_HANDLER:
    PUSH R16
    IN R16, SREG
    PUSH R16
    LDS R16, LockState
    CPI R16, 1
    BREQ exit_int0
    CPI CurrentDisplay, 3
    BREQ check_pin_call
    INC CurrentDisplay
    CPI CurrentDisplay, 4
    BRNE exit_int0
    CLR CurrentDisplay
    RJMP exit_int0
check_pin_call:
    RCALL check_pin
exit_int0:
    POP R16
    OUT SREG, R16
    POP R16
    RETI

; Обработчик INT1
INT1_HANDLER:
    PUSH R16
    IN R16, SREG
    PUSH R16
    LDS R16, LockState
    CPI R16, 1
    BREQ exit_int1
    DEC CurrentDisplay
    CPI CurrentDisplay, 0xFF
    BRNE exit_int1
    LDI CurrentDisplay, 3
exit_int1:
    POP R16
    OUT SREG, R16
    POP R16
    RETI

TIMER1_COMPA:
    PUSH R16
    IN R16, SREG
    PUSH R16
    PUSH R17
    PUSH R2 ; Используется для LockCounter
    PUSH R3 ; Используется для LockCounter

    ; Очистка дисплея, сохраняя PA6 и PA7
    IN R16, PORTA
    ANDI R16, 0b11000000 ; ANDI - логическое И с константой, оставляем только биты PA6 и PA7
    OUT PORTA, R16
    CLR R16
    OUT PORTC, R16

    ; Проверка состояния блокировки
    LDS R16, LockState
    CPI R16, 1
    BRNE not_locked
    RJMP locked_display

not_locked:
    ; Считываем состояние PIND
    IN PinD_, PIND
    ; Проверка PD0 (кнопка инкремента)
    SBIC PIND, 0
    RJMP pd0_released ; Если PD0 = 1 (отпущена), переходим к pd0_released
    ; PD0 = 0 (кнопка нажата)
    SBR Button0Pressed, 1 ; Устанавливаем флаг нажатия кнопки PD0
    ; Увеличиваем счётчик удержания для PD0
    LDS R16, ButtonHoldCounterPD0
    INC R16
    STS ButtonHoldCounterPD0, R16
    ; Проверяем этап удержания
    LDS R17, HoldStagePD0
    CPI R17, 0 
    BREQ pd0_first_stage
    ; Второй этап (ещё 1 секунда)
    CPI R16, 200
    BRLO check_pd1
    ; 2 секунды прошло, активируем автоинкремент
    LDI R16, 1
    STS AutoIncrementActive, R16
    ; Сбрасываем счётчик для следующего цикла
    CLR R16
    STS ButtonHoldCounterPD0, R16
    RJMP check_pd1
pd0_first_stage:
    ; Первый этап (1 секунда)
    CPI R16, 200
    BRLO check_pd1
    ; 1 секунда прошла, переходим на второй этап
    LDI R17, 1
    STS HoldStagePD0, R17
    CLR R16
    STS ButtonHoldCounterPD0, R16
    RJMP check_pd1

pd0_released:
    SBRS Button0Pressed, 0
    RJMP pd0_not_held
    ; Кнопка отпущена и флаг был установлен
    CBR Button0Pressed, 1
    RCALL increment_digit
pd0_not_held:
    ; Сбрасываем счётчик, этап удержания и флаг автоинкремента для PD0
    CLR R16
    STS ButtonHoldCounterPD0, R16
    STS HoldStagePD0, R16
    STS AutoIncrementActive, R16
  

check_pd1:
    ; Проверка PD1 (кнопка декремента)
    SBIC PIND, 1
    RJMP pd1_released
    ; PD1 = 0 (кнопка нажата)
    SBR Button1Pressed, 1
    ; Увеличиваем счётчик удержания для PD1
    LDS R16, ButtonHoldCounterPD1
    INC R16
    STS ButtonHoldCounterPD1, R16
    ; Проверяем этап удержания
    LDS R17, HoldStagePD1
    CPI R17, 0
    BREQ pd1_first_stage
    ; Второй этап (ещё 1 секунда)
    CPI R16, 200
    BRLO skip_hold_action
    ; 2 секунды прошло, включаем PA7 и активируем автодекремент
    LDI R16, 1
    STS AutoDecrementActive, R16
    ; Сбрасываем счётчик для следующего цикла
    CLR R16
    STS ButtonHoldCounterPD1, R16
    RJMP skip_hold_action
pd1_first_stage:
    ; Первый этап (1 секунда)
    CPI R16, 200
    BRLO skip_hold_action
    ; 1 секунда прошла, переходим на второй этап
    LDI R17, 1
    STS HoldStagePD1, R17
    CLR R16
    STS ButtonHoldCounterPD1, R16
    RJMP skip_hold_action

pd1_released:
    SBRS Button1Pressed, 0
    RJMP pd1_not_held
    ; Кнопка отпущена и флаг был установлен
    CBR Button1Pressed, 1
    RCALL decrement_digit
pd1_not_held:
    ; Сбрасываем счётчик, этап удержания и флаг автодекремента для PD1
    CLR R16
    STS ButtonHoldCounterPD1, R16
    STS HoldStagePD1, R16
    STS AutoDecrementActive, R16

skip_hold_action:
    ; Выбор текущего индикатора
    LDS R17, CurrentDigitDisplay
    LDI R16, 0b00000001 ; Начальная маска для PA0 (первый индикатор)
    CPI R17, 0 ; Если CurrentDigitDisplay = 0, пропускаем сдвиг
    BREQ skip_shift
shift_loop:
    LSL R16 ; Сдвигаем бит влево для выбора следующего индикатора (PA1, PA2, PA3)
    DEC R17 ; Уменьшаем счётчик
    BRNE shift_loop ; Повторяем, пока не выберем нужный индикатор
skip_shift:
    IN R17, PORTA
    ANDI R17, 0b11000000 ; Сохраняем состояния PA6 и PA7 (индикаторы ошибок)
    OR R16, R17 ; Объединяем маску индикатора с сохранёнными битами PA6 и PA7
    OUT PORTA, R16 ; Выводим на PORTA для активации текущего индикатора

    ; Загрузка и вывод цифры с учётом мигания
    LDS R17, CurrentDigitDisplay
    CPI R17, 0
    BREQ load_digit0
    CPI R17, 1
    BREQ load_digit1
    CPI R17, 2
    BREQ load_digit2
    MOV R16, Digit3
    RCALL show_digit
    RJMP display_digit
load_digit0:
    MOV R16, Digit0
    RCALL show_digit
    RJMP display_digit
load_digit1:
    MOV R16, Digit1
    RCALL show_digit
    RJMP display_digit
load_digit2:
    MOV R16, Digit2
    RCALL show_digit
display_digit:
    CP R17, R18 ; Сравниваем CurrentDigitDisplay с CurrentDisplay (текущий редактируемый разряд)
    BRNE static_digit ; Если не совпадают, выводим статично
    SBRC Blink, 0 ; Если совпадают и бит мигания = 1, выводим цифру
    OUT PORTC, R16
    RJMP update_digit
static_digit:
    OUT PORTC, R16
    RJMP update_digit

locked_display:
    IN R16, PORTA
    ANDI R16, 0b10000000
    ORI R16, 0b01000000
    OUT PORTA, R16

update_digit:
    ; Увеличение CurrentDigitDisplay
    LDS R17, CurrentDigitDisplay
    INC R17
    CPI R17, 4
    BREQ met
	RJMP store_digit
met:
    CLR R17
    ; Управление миганием и автоинкремент/автодекремент
    LDS R16, LockState
    CPI R16, 1
    BREQ skip_blink
    LDS R16, BlinkCounter
    INC R16
    STS BlinkCounter, R16
    CPI R16, 25
    BRNE no_blink_toggle
    ; Полный цикл мигания (каждые 125 мс)
    LDI R16, 0b00000001 ; Маска для переключения бита
    EOR Blink, R16 ; Инвертируем бит мигания (EOR = XOR)
    CLR R16
    STS BlinkCounter, R16
    ; Проверяем автоинкремент (PD0)
    LDS R16, AutoIncrementActive
    CPI R16, 1
    BRNE check_auto_decrement
    ; Выполняем автоинкремент, только если Blink = 1 (каждые 250 мс)
    SBRS Blink, 0
    RJMP check_auto_decrement
    RCALL increment_digit
check_auto_decrement:
    ; Проверяем автодекремент (PD1)
    LDS R16, AutoDecrementActive
    CPI R16, 1
    BRNE no_blink_toggle
    ; Выполняем автодекремент, только если Blink = 1 (каждые 250 мс)
    SBRS Blink, 0
    RJMP no_blink_toggle
    RCALL decrement_digit
no_blink_toggle:
    STS BlinkCounter, R16 ; Сохраняем значение счётчика (пока не достиг 25)
skip_blink:

    ; Управление задержкой
    LDS R16, LockState
    CPI R16, 1
    BRNE store_digit
    ; Увеличение LockCounter
    LDS R2, LockCounter
    LDS R3, LockCounter+1
    LDI R16, 1
    ADD R2, R16
    CLR R16
    ADC R3, R16
    STS LockCounter, R2
    STS LockCounter+1, R3
    ; Проверка на 1000 (20 секунд)
    LDI R16, LOW(1000)
    CP R2, R16
    BRNE skip_reset
    LDI R16, HIGH(1000)
    CP R3, R16
    BRNE skip_reset
    ; Сброс состояния
    CLR R16
    STS LockState, R16
    STS LockCounter, R16
    STS LockCounter+1, R16
    CBI PORTA, 6
    RCALL reset_display
skip_reset:

store_digit:
    STS CurrentDigitDisplay, R17

    POP R3
    POP R2
    POP R17
    POP R16
    OUT SREG, R16
    POP R16
    RETI

; Проверка PIN-кода
check_pin:
    CP Digit0, PinDigit0
    BRNE pin_incorrect
    CP Digit1, PinDigit1
    BRNE pin_incorrect
    CP Digit2, PinDigit2
    BRNE pin_incorrect
    CP Digit3, PinDigit3
    BRNE pin_incorrect
    RCALL pin_correct
    RET

pin_incorrect:
    INC IncorrectAttempts
    CPI IncorrectAttempts, 3
    BRGE pin_locked
    LDI R16, 1
    STS LockState, R16
    CLR R16
    STS LockCounter, R16
    STS LockCounter+1, R16
    SBI PORTA, 6
    CBI PORTA, 0
    CBI PORTA, 1
    CBI PORTA, 2
    CBI PORTA, 3
    RET

pin_correct:
    SBI PORTA, 7
    CBI PORTA, 0
    CBI PORTA, 1
    CBI PORTA, 2
    CBI PORTA, 3
    LDI R16, 0xC0
    IN R17, GICR
    AND R17, R16
    OUT GICR, R17
    CLI
lock_loops:
    RJMP lock_loops

pin_locked:
    LDI R19, 3
blink_loop:
	CBI PORTA, 0
    CBI PORTA, 1
    CBI PORTA, 2
    CBI PORTA, 3
	CLR R16
	OUT PORTC, R16
    SBI PORTA, 7
    SBI PORTA, 6
    RCALL blink_locked
    CBI PORTA, 7
    CBI PORTA, 6
    RCALL blink_locked
    DEC R19
    BRNE blink_loop
    CLR Blink
    CLI
lock_loop:
    RJMP lock_loop

; Сброс дисплея
reset_display:
    CLR Digit0
    CLR Digit1
    CLR Digit2
    CLR Digit3
    CLR CurrentDisplay
    RET

blink_locked:
    LDI R20, 100
outers:
    LDI R21, 80
middles:
    LDI R22, 100
inners:
    DEC R22
    BRNE inners
    DEC R21
    BRNE middles
    DEC R20
    BRNE outers
    RET
