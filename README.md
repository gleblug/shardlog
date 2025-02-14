# �������� ��������� shardlog
�������������� ������ ���������-�����������.

1. [��������](#introduction)
1. [�������������](#usage)
1. [�������� �������](#functions)
1. [������������](#configs)
    - [���� `config.ini`](#configs-ini)
    - [���� `commands.yaml`](#configs-commands)

## �������� <a name="introduction"></a>
��������� shardlog �������� �� ����� C++ � ������������� ��� ��������������� ����� ������ � ��������� �����������. ��� ������� � ������������ ������������� ������������� ����������� NI-Visa USB ��� COM.

## ������������� <a name="usage"></a>
**�����������:**
- �������� NI Visa.

��� ������������� ��������� ���������� �������� ���������������� ����� `config.ini` � `commands.yaml` � ����� � ����������, � ����� ��������� ���� `shardlog.exe`. ����� �� ���������� ���������� ����� ����������� � ���������� ����.

## �������� ������� <a name="functions"></a>
�������� ����������������� ��������� �������� ���������� ����� ������������ ����������� ���� ��� �� ������������� ��������� �������� *timeout*. ��� �������������� � ������������ ������������ ����� ������������, � ������� ����������� ����, ������� �, ��� �������������, �������� ��� ������.

���� ���������� �� ����� ���� ����� �� ����� *timeout*, ����� �������� ���������� �������, � ����� ������ �� ����� ������ �� ��������� ������ ��� ��������� �������� ����������. ��� ������� ���������� (NI-Visa, COM) � ��������� ����� ���� �������.

## ������������ <a name="configs"></a>
������������ ������� ������������ ��� ���������� ������ ��������� �������� `config.ini` � `commands.yaml`. ������� ������ ������������ ����� ������� � ����� [examples](examples/). ��� ������� ���������� ����� ��������� ��������� ���� `<table_name>.csv`, �� �������� ����� ������� �������� ��� ������. ��������, ��� ��������� ���������� ��� ���� �� ��������� �����.

��� ���� �������� ������������� ��-���������, ���� �� ������� ���������.

### ���� `config.ini` <a name="configs-ini"></a>
#### ������ [measurer]
����� ����������� ������������ ����������:
- `meters` � ����� ������������ �����������, ����� ������.
- `directory` � ���������� ��� ���������� ������ ������������.
- `duration` � ����� ������������ ������������, � ��������.
- `timeout` � ������� ������ �����������. �� ������ 0.05 �.

#### ������ ����������� [meter.<METER_NAME>]
`METER_NAME` � ����� **����������** ��� ����������.
- `commands` � �������� �������� ������ �� ����� `commands.yaml`.
- `port` � ���� ����������.
- `setValues` (�����������) � ��� ����� �� ���������� ��� ������.

### ���� `commands.yaml` <a name="configs-commands"></a>
� ���� ����� ����������� �������, � ������� ������� ����� ����� �������� ��������� � ����������.
- `name` (�����������) � ���������� ��� ������ ������.
- `configure` (�����������) � �������, ������� ����������� �� ������ **����** ��� ����� ������� ���������.
- `read` � ������������� ������ ������ ��� ��������� �������, ����������� �� ������ ��� ������ ���������:
    - `<VALUE_ONE>` � ����� ������ ��� ��������� ������ ��������.
    - `<VALUE_TWO>` - ����� ������ ��� ��������� ������ ��������.
    - `<VALUE_N>` � ...
- `set` (�����������) � ������� ��� ��������� �������� �� ����� `setValues` �� ��������� �����. �������� ������� ��� `{<IDX>}`.
- `end` (�����������) � ������� ��� ���������� ������ ������ � �����������, ������������ **����** ��� � ����� ������������.