# SumoControllerQt

Aplikacja Qt 6 demonstracyjna do obsługi Bluetooth Low Energy (BLE) na systemach Windows oraz Android. Program pozwala wyszukiwać urządzenia BLE, nawiązywać połączenie oraz wysyłać i odbierać dane z wykorzystaniem buforów tablicowych.

## Funkcjonalności

- Skanowanie urządzeń BLE i wyświetlanie listy znalezionych nadajników.
- Łączenie i rozłączanie z wybranym urządzeniem.
- Konfigurowanie identyfikatorów UUID usługi oraz charakterystyk TX/RX.
- Wysyłanie danych wpisanych jako lista liczb całkowitych (CSV) do bufora TX i przesyłanie do urządzenia.
- Odbieranie danych z charakterystyki notyfikacji/odczytu i prezentowanie ich w buforze RX.
- Logowanie zdarzeń komunikacji BLE.

## Wymagania

- Qt 6.2 lub nowsze z modułami `Qt::Core`, `Qt::Gui`, `Qt::Widgets` oraz `Qt::Bluetooth`.
- Kompilator zgodny ze standardem C++17.
- Na Androidzie konieczne są odpowiednie uprawnienia BLE w plikach manifestu (generowane automatycznie przez Qt).

## Budowanie projektu

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Na Androidzie należy użyć narzędzi `cmake`/`ninja` dostarczanych z Qt oraz skonfigurować zestaw narzędzi Androida zgodnie z dokumentacją Qt.

## Użytkowanie

1. Uruchom aplikację i wybierz przycisk **Skanuj**, aby wyszukać urządzenia BLE.
2. Zaznacz wybrane urządzenie na liście i kliknij **Połącz**.
3. W razie potrzeby zmodyfikuj identyfikatory UUID usługi i charakterystyk w formularzu.
4. Wprowadź dane do wysłania jako listę liczb (np. `1,2,3,4`) i wybierz **Wyślij**, aby przesłać dane.
5. Odebrane dane pojawią się w sekcji **Odebrane dane** oraz zostaną zapisane w wewnętrznym buforze.

> **Uwaga:** Działanie aplikacji zależy od dostępności fizycznego urządzenia BLE oraz odpowiednich uprawnień systemowych. W środowisku deweloperskim bez urządzenia BLE funkcjonalności komunikacyjne nie będą aktywne.
