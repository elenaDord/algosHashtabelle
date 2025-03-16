#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1031  // Primzahl als Tabellengröße für weniger Kollisionen
#define MAX_KURSE 30     // Maximale Anzahl von Kursdaten
#define DELETED ((Aktie*)-1) //DELETED markiert einen ungültigen Zeige (Aktie* wird auf -1 gecastet)

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

int getKursdaten(char filename[], kursdaten kurse[])
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Fehler: Datei %s konnte nicht geöffnet werden.\n", filename);
        return 0;
    }

    char line[100];
    int count = 0;

    // Erste Zeile (Header) überspringen
    fgets(line, sizeof(line), file);

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

    while (hashtable[(index + i * i) % TABLE_SIZE] != NULL && i <= TABLE_SIZE) {
        Aktie *aktie = hashtable[(index + i * i) % TABLE_SIZE];

        if (aktie == DELETED) {
            i++;
            continue;
        }
        if (searchByName) {
            if (strcmp(aktie->name, key) == 0) {
                return aktie;
            }
        }
        else {
            if (strcmp(aktie->kuerzel, key) == 0) {
                return aktie;
            }
        }

        i++;
    }

    return NULL;
}

void importKursdaten(Aktie *hashtable[])
{
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

void deleteAktie(Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    char name[20];
    printf("\nBitte geben Sie den Namen der Aktie ein, die Sie löschen möchten:\n");
    scanf(" %19[^\n]", name);

    int indexName = hashFunction(name);
    int i = 0;
    int foundIndex = -1;
    char kuerzel[10];

    // Durchsuche hashtableName nach der Aktie und speichere Index + Kürzel
    while (hashtableName[(indexName + i * i) % TABLE_SIZE] != NULL && hashtableName[(indexName + i * i) % TABLE_SIZE] != DELETED && i <= TABLE_SIZE){
        Aktie *aktie = hashtableName[(indexName + i * i) % TABLE_SIZE];

        if (aktie != DELETED && strcmp(aktie->name, name) == 0) {
            foundIndex = (indexName + i * i) % TABLE_SIZE;
            strcpy(kuerzel, aktie->kuerzel);
            break;
        }
        i++;
    }

    // Falls nicht gefunden
    if (foundIndex == -1) {
        printf("Fehler: Aktie '%s' nicht gefunden!\n", name);
        return;
    }

    // Löschen aus der Namens-Hashtabelle
    free(hashtableName[foundIndex]);
    hashtableName[foundIndex] = DELETED;

    // Löschen aus der Kürzel-Hashtabelle
    int indexKrz = hashFunction(kuerzel);
    i = 0;
    while (hashtableKrz[(indexKrz + i * i) % TABLE_SIZE] != NULL && i <= TABLE_SIZE) {
        if (hashtableKrz[(indexKrz + i * i) % TABLE_SIZE] != DELETED &&
            strcmp(hashtableKrz[(indexKrz + i * i) % TABLE_SIZE]->kuerzel, kuerzel) == 0) {
            free(hashtableKrz[(indexKrz + i * i) % TABLE_SIZE]);
            hashtableKrz[(indexKrz + i * i) % TABLE_SIZE] = DELETED;
            break;
        }
        i++;
    }

    printf("Aktie '%s' wurde erfolgreich gelöscht.\n", name);
}

// Aktie in Hash-Tabelle einfügen (quadratische Sondierung)
int insertIntoHashTable(Aktie *hashtable[], char *key, Aktie *aktie)
{
    int index = hashFunction(key);
    int i = 0;

    while (hashtable[(index + i * i) % TABLE_SIZE] != NULL && hashtable[(index + i * i) % TABLE_SIZE] != DELETED){
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
    neueAktie->kursIndex = 0;

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
    printf("Wählen Sie eine Option:\n1: Aktie hinzufügen\n2: Aktie löschen\n3: Kursverlauf importieren\n4: Aktie suchen\n5: Plot ausgeben\n6: Hashtabelle in eine Datei speichern\n7: Hashtabelle aus einer Datei laden\n8: Program beenden\n");
    scanf(" %d", &input);
    return input;
}

void userInputNewAktie(Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    char newStockName[20];
    char newStockSymbol[10];
    int newStockNumber;

    printf("\nNeue Aktie hinuzfügen! \nBitte geben Sie den Namen der Aktie ein: ");
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
    }
}

void saveHashTable(char *filename, Aktie *hashtable[]) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Fehler beim Speichern!\n");
        return;
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashtable[i] && hashtable[i] != DELETED) {
            fwrite(hashtable[i], sizeof(Aktie), 1, file);
        }
    }

    fclose(file);
    printf("Daten erfolgreich gespeichert.\n");
}

void loadHashTable(char *filename, Aktie *hashtableName[], Aktie *hashtableKrz[]) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Fehler beim Laden!\n");
        return;
    }

    // Alte Hashtabellen leeren
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashtableName[i] && hashtableName[i] != DELETED) {
            free(hashtableName[i]);
        }
        hashtableName[i] = NULL;

        if (hashtableKrz[i] && hashtableKrz[i] != DELETED) {
            hashtableKrz[i] = NULL; // Kein `free`, da beide Tabellen auf dieselbe Aktie zeigen
        }
    }

    Aktie temp;
    while (fread(&temp, sizeof(Aktie), 1, file)) {
        Aktie *neueAktie = malloc(sizeof(Aktie));
        if (!neueAktie) {
            printf("Speicherfehler!\n");
            fclose(file);
            return;
        }
        *neueAktie = temp;

        // Einfügen in beide Hashtabellen
        int inserted1 = insertIntoHashTable(hashtableName, neueAktie->name, neueAktie);
        int inserted2 = insertIntoHashTable(hashtableKrz, neueAktie->kuerzel, neueAktie);

        if (!inserted1 && !inserted2) {
            free(neueAktie);
            printf("Fehler beim Laden einer Aktie!\n");
        }
    }

    fclose(file);
    printf("Daten erfolgreich geladen.\n");
}

void plotKursdaten(Aktie *hashtable[]) {
    char searchTerm[20];
    printf("\nBitte geben Sie den Namen der Aktie ein: ");
    scanf(" %19[^\n]", searchTerm);

    Aktie *aktie = searchAktie(hashtable, searchTerm, 1);

    if (!aktie) {
        printf("Fehler: Aktie nicht gefunden!\n");
        return;
    }

    if (aktie->kursIndex == 0) {
        printf("Keine Kursdaten vorhanden!\n");
        return;
    }

    // Höchsten und niedrigsten Schlusskurs finden
    float minClose = aktie->kurse[0].close;
    float maxClose = aktie->kurse[0].close;

    for (int i = 1; i < aktie->kursIndex; i++) {
        if (aktie->kurse[i].close < minClose) minClose = aktie->kurse[i].close;
        if (aktie->kurse[i].close > maxClose) maxClose = aktie->kurse[i].close;
    }

    float range = maxClose - minClose;
    if (range == 0) range = 1;  // Vermeidung von Division durch 0

    printf("\nSchlusskurse der letzten %d Tage für %s:\n", aktie->kursIndex, aktie->name);

    // ASCII-Plot zeichnen
    for (int i = 0; i < aktie->kursIndex; i++) {
        int stars = (int) ((aktie->kurse[i].close - minClose) / range * 50);  // Skalierung auf 50 Zeichen
        printf("%s | ", aktie->kurse[i].date);
        for (int j = 0; j < stars; j++) {
            printf("*");
        }
        printf(" %.2f\n", aktie->kurse[i].close);
    }
}

void handleUserInput(Aktie *hashtableName[], Aktie *hashtableKrz[])
{
    int input = 0;
    while (input != 8) {
        input = getUserInput();

        switch (input) {
            case 1:
                userInputNewAktie(hashtableName, hashtableKrz);
                break;
            case 2:
                deleteAktie(hashtableName, hashtableKrz);
                break;
            case 3:
                importKursdaten(hashtableName);
                break;
            case 4:
                handleSearch(hashtableName, hashtableKrz);
                break;
            case 5:
                plotKursdaten(hashtableName);
                break;
            case 6:
                printf("Dateiname zum Speichern: ");
                char saveFile[40];
                scanf(" %39[^\n]", saveFile);
                saveHashTable(saveFile, hashtableName);
                break;
            case 7:
                printf("Dateiname zum Laden: ");
                char loadFile[40];
                scanf(" %39[^\n]", loadFile);
                loadHashTable(loadFile, hashtableName, hashtableKrz);
                break;
            case 8:
                printf("Programm beendet.\n");
                break;
            default:
                printf("Ungültige Eingabe!\n");
        }
    }
}

/*
void printHashTable(Aktie *hashtable[]) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hashtable[i] && hashtable[i] != DELETED) {
            printf("Name: %s, Kürzel: %s, WKN: %d\n", hashtable[i]->name, hashtable[i]->kuerzel, hashtable[i]->WKN);
        }
    }
}
*/

int main() {
    Aktie *hashtableName[TABLE_SIZE] = {NULL};  // Alle Einträge auf NULL setzen
    Aktie *hashtableKrz[TABLE_SIZE] = {NULL};  // Alle Einträge auf NULL setzen
    handleUserInput(hashtableName, hashtableKrz);
    for (int i = 0; i < TABLE_SIZE; i++) {
    if (hashtableName[i] && hashtableName[i] != DELETED) {
        free(hashtableName[i]);
    }
    }
    return 0;
}
