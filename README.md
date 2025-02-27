# Описание программы shardlog
Автоматический логгер устройств-измерителей.

1. [Введение](#introduction)
1. [Использование](#usage)
1. [Основные функции](#functions)
1. [Конфигурация](#configs)
    - [Файл `config.ini`](#configs-ini)
    - [Файл `commands.yaml`](#configs-commands)

## Введение <a name="introduction"></a>
Программа shardlog написана на языке C++ и предназначена для автоматического сбора данных с различных измерителей. Для общения с измерителями предусмотрено использование интерфейсов NI-Visa USB или COM.

## Использование <a name="usage"></a>
**Зависимости:**
- Драйвера NI Visa.

Для использования программы необходимо положить конфигурационные файлы `config.ini` и `commands.yaml` в папку с программой, а затем запустить файл `shardlog.exe`. Далее всё управление программой будет происходить в консольном окне.

## Основные функции <a name="functions"></a>
Основной функциональностью программы является постоянный опрос подключённых измерителей один раз за фиксированный временной интервал *timeout*. Для взаимодействия с измерителями используются файлы конфигурации, в которых описывается порт, команды и, при необходимости, значения для записи.

Если измеритель не успел дать ответ за время *timeout*, будет записано предыдущее значние, а новый запрос не будет послан до получения ответа или истечения таймаута соединения. Для каждого соедниения (NI-Visa, COM) в программе задан свой таймаут.

## Конфигурация <a name="configs"></a>
Необходимыми файлами конфигурации для корректной работы программы являются `config.ini` и `commands.yaml`. Примеры файлов конфигурации можно увидеть в папке [examples](examples/). Для каждого измерителя можно создавать отдельный файл `<table_name>.csv`, из которого будут браться значения для записи. Например, для установки напряжения или тока по временной метке.

Все поля являются обязательными по-умолчанию, если не сказано обратного.

### Файл `config.ini` <a name="configs-ini"></a>
#### Секция [measurer]
Здесь описывается конфигурация измерителя:
- `meters` — имена используемых измерителей, через пробел.
- `directory` — директория для сохранения данных эксперимента.
- `duration` — общая длительность эксперимента, в секундах.
- `timeout` — таймаут опроса измерителей. Не меньше 0.05 с.

#### Секции измерителей [meter.<METER_NAME>]
`METER_NAME` — любое **уникальное** имя измерителя.
- `commands` — название паттерна команд из файла `commands.yaml`.
- `port` — порт измерителя.
- `setValues` (опицонально) — имя файла со значениями для записи.

### Файл `commands.yaml` <a name="configs-commands"></a>
В этом файле описываются команды, с помощью которых между собой общаются программа и измерители.
- `name` (опционально) — уникальное имя набора команд.
- `configure` (опционально) — команды, которые отпраляются на прибор **один** раз перед началом измерения.
- `read` — ассоциативный массив команд для измерения величин, отпраляются на прибор при каждом измерении:
    - `<VALUE_ONE>` — набор команд для измерения первой величины.
    - `<VALUE_TWO>` - набор команд для измерения второй величины.
    - `<VALUE_N>` — ...
- `set` (опционально) — команды для установки значения из файла `setValues` по временной метке. Аргумент задаётся как `{<IDX>}`.
- `end` (опционально) — команды для завершения сессии работы с измерителем, отправлюятся **один** раз в конце эксперимента.
