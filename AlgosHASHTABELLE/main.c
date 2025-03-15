#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1031  // Primzahl als Tabellengröße für weniger Kollisionen
#define MAX_KURSE 30     // Maximale Anzahl von Kursdaten

typedef struct{
    char date[11];
    int volume;
    float open;
    float close;
    float high;
    float low;
}kursdaten;

typedef struct{
    char name[20];
    char kuerzel[10];
    int WKN;
    kursdaten kurse[MAX_KURSE];  // Array mit 30 Kursdaten
    int kursIndex;     // Anzahl gespeicherter Kursdaten
} Aktie;

int hashFunction(char *key)
{
    int sum = 0;
    while (*key) {
        sum += (*key);  // ASCII-Wert aufaddieren
        key++;
    }
    return sum % TABLE_SIZE;
}

int getKursdaten(char filename[], kursdaten *kurse)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Fehler: Datei %s konnte nicht geöffnet werden.\n", filename);
        return 0;
    }

    char line[100];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < MAX_KURSE){
        kursdaten kd;
        sscanf(line, "%10[^,],$%f,%d,$%f,$%f,$%f", kd.date, &kd.close, &kd.volume, &kd.open, &kd.high, &kd.low);
        kurse[count++] = kd;
    }

    fclose(file);
    return count; // Anzahl gelesener Datensätze
}

Aktie* searchAktie(Aktie *hashtable[], char *key, int searchByName)
{
    int index = hashFunction(key);
    int i = 0;

    while (hashtable[(index + i * i) % TABLE_SIZE] != NULL) {
        Aktie *aktie = hashtable[(index + i * i) % TABLE_SIZE];

        if (searchByName) {
            if (strcmp(aktie->name, key) == 0) {
                return aktie;
            }
        } else {
            if (strcmp(aktie->kuerzel, key) == 0) {
                return aktie;
            }
        }

        i++;
        if (i >= TABLE_SIZE) break;
    }

    return NULL;
}

void importKursdaten(Aktie *hashtable[]){
    char searchTerm[20];
    printf("\nBitte geben Sie den Namen der Aktie an, der Sie die CSV-Datei hinzufügen wollen:\n");
    scanf(" %19[^\n]", searchTerm);

    Aktie *result = searchAktie(hashtable, searchTerm, 1);
    if (!result) {
        printf("Fehler: Aktie '%s' nicht gefunden!\n", searchTerm);
        return;
    }

    char csvDatei[40];
    printf("\nBitte geben Sie den Pfad der CSV-Datei an:\n");
    scanf(" %39[^\n]", csvDatei);

    result->kursIndex = getKursdaten(csvDatei, result->kurse);
}

// Aktie in Hash-Tabelle einfügen (quadratische Sondierung)
int insertIntoHashTable(Aktie *hashtable[], char *key, Aktie *aktie)
{
    int index = hashFunction(key);
    int i = 0;

    while (hashtable[(index + i * i) % TABLE_SIZE] != NULL) {
        i++;
        if (i >= TABLE_SIZE) {
            printf("Hashtabelle ist voll!\n");
            return 0;
        }
    }

    hashtable[(index + i * i) % TABLE_SIZE] = aktie;
    return 1;
}

// Funktion zum Hinzufügen einer Aktie
void addAktie(char name[], char kuerzel[], int WKN, Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    Aktie* neueAktie = malloc(sizeof(Aktie));
    if (!neueAktie) {
        printf("Speicherzuweisung fehlgeschlagen!\n");
        return;
    }

    strcpy(neueAktie->name, name);
    strcpy(neueAktie->kuerzel, kuerzel);
    neueAktie->WKN = WKN;
    //neueAktie->kursIndex = importKursdaten(csvDatei, neueAktie->kurse);

    if (neueAktie->kursIndex == 0) {
        printf("Fehler beim Importieren der Kursdaten.\n");
        free(neueAktie);
        return;
    }

    // Einfügen in beide Hashtabellen (Name & Kürzel)
    int inserted1 = insertIntoHashTable(hashtableName, name, neueAktie);
    int inserted2 = insertIntoHashTable(hashtableKrz, kuerzel, neueAktie);

    if (!inserted1 && !inserted2) {  // Nur wenn beide fehlschlagen!
        free(neueAktie);
        printf("Fehler beim Einfügen in die Hashtabelle!\n");
    }
    else {
        printf("Aktie '%s' (Kürzel: %s) erfolgreich hinzugefügt.\n", name, kuerzel);
    }
}

void printAktie(char* searchTerm, Aktie *hashtable[], int searchByName)
{
    Aktie *result = searchAktie(hashtable, searchTerm, searchByName);
    if (result == NULL) {
        printf("Aktie nicht gefunden!\n");
        return;
    }
    printf("Name: %s\nKürzel: %s\nWKN: %d\n", result->name, result->kuerzel, result->WKN);
    for(int i = 0; i < result->kursIndex; i++){
        kursdaten kd = result->kurse[i];
        printf("Datum: %s, Open: %.2f, Close: %.2f, High: %.2f, Low: %.2f, Volume: %d\n",
            kd.date, kd.open, kd.close, kd.high, kd.low, kd.volume);
    }
}

int getUserInput()
{
    int input;
    printf("Wählen Sie eine Option:\n1: Aktie hinzufügen\n2: Aktie löschen\n3: Aktie importieren\n4: Aktie suchen\n");
    scanf(" %d", &input);
    return input;
}

void userInputNewAktie(Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    char newStockName[20];
    char newStockSymbol[10];
    int newStockNumber;

    printf("\n Neue Aktie hinuzfügen! \nBitte geben Sie den Namen der Aktie ein: ");
    scanf(" %[^\n]", newStockName);
    printf("\nBitte geben Sie das Kürzel der Aktie ein: ");
    scanf(" %[^\n]", newStockSymbol);
    printf("\nBitte geben Sie die WKN der Aktie ein: ");
    scanf(" %d", &newStockNumber);
    addAktie(newStockName, newStockSymbol, newStockNumber, hashtableName, hashtableKrz);
}

int getSearchItem()
{
    int choice;
    printf("\nMöchten Sie nach Name (1) oder Kürzel (2) suchen? ");
    scanf("%d", &choice);
    if(choice != 1 && choice !=2){
       printf("\nUngültige Eingabe!");
       return 0;
    }
    return choice;
}

void handleSearch(Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    int choice = getSearchItem(); //1:Name, 2:Kürzel
    char searchTerm[20];
    printf("\nBitte geben Sie den Suchbegriff ein: ");
    scanf(" %19[^\n]", searchTerm);

    if (choice == 1) {
        printAktie(searchTerm, hashtableName, 1);
    } else if (choice == 2) {
        printAktie(searchTerm, hashtableKrz, 0);
    } else {
        printf("Ungültige Eingabe!\n");
    }
}

void handleUserInput(Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    int input = 0;
    while (input != 8) {
        input = getUserInput();  // Korrektur: Ohne `int` neu zu deklarieren!

        switch (input) {
            case 1:
                userInputNewAktie(hashtableName, hashtableKrz);
                break;
            case 3:
                importKursdaten(hashtableName);
                break;
            case 4:
                handleSearch(hashtableName, hashtableKrz);
                break;
            case 8:
                printf("Programm beendet.\n");
                break;
            default:
                printf("Ungültige Eingabe!\n");
        }
    }
}

int main() {
    Aktie *hashtableName[TABLE_SIZE] = {NULL};  // Alle Einträge auf NULL setzen
    Aktie *hashtableKrz[TABLE_SIZE] = {NULL};  // Alle Einträge auf NULL setzen
    handleUserInput(hashtableName, hashtableKrz);
    return 0;
}
