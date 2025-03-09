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
    char name[50];
    char kuerzel[10];
    char WKN[10];
    kursdaten kurse[MAX_KURSE];  // Array mit 30 Kursdaten
    int kursIndex;     // Anzahl gespeicherter Kursdaten
} Aktie;

// Globale Hashtabelle
Aktie *hashtable[TABLE_SIZE] = {NULL};  // Alle Einträge auf NULL setzen



int hashFunction(char *key)
{
    int sum = 0;
    while (*key) {
        sum += (*key);  // ASCII-Wert aufaddieren
        key++;
    }
    return sum % TABLE_SIZE;
}

int importKursdaten(char filename[], kursdaten *kurse)
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

// Funktion zum Hinzufügen einer Aktie zur Hashtabelle
void addAktie(char name[], char kuerzel[], char WKN[], char csvDatei[])
{
    Aktie* neueAktie = malloc(sizeof(Aktie));
    if (!neueAktie) {
        printf("Speicherzuweisung fehlgeschlagen!\n");
        return;
    }

    strcpy(neueAktie->name, name);
    strcpy(neueAktie->kuerzel, kuerzel);
    strcpy(neueAktie->WKN, WKN);
    neueAktie->kursIndex = importKursdaten(csvDatei, neueAktie->kurse);

    if (neueAktie->kursIndex == 0) {
        printf("Fehler beim Importieren der Kursdaten.\n");
        free(neueAktie);
        return;
    }

    int index = hashFunction(name);

    int i=0;
    while(hashtable[(index + i*i)%TABLE_SIZE] != NULL){
        i++;
        if(i > TABLE_SIZE){
            free(neueAktie);
            printf("Hashtabelle ist voll!\n");
        }
        hashtable[(index + i*i)%TABLE_SIZE] = neueAktie;
        printf("Aktie '%s' (Kürzel: %s) erfolgreich hinzugefügt.\n", name, kuerzel);

    }

}

// Beispielmain-Funktion
int main() {
     addAktie("Apple Inc.", "AAPL", "865985", "apple.csv");

    return 0;
}
