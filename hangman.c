#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>

// Length of longest word
#define WORDLEN 45

// Length of alphabet
#define ALPHABET 26

// File with word list
#define FILENAME "words.txt"

typedef struct node
{
    char word[WORDLEN + 1];
    struct node *next;
}
node;

// Function prototypes
char *get_input(char *buff, size_t size, size_t *inplen);
char get_char();
int load_words(node **rootNode);
char *get_random_word(node *rootNode, int wordCount);
bool play(char *word, int wordlen);
int word_to_upper(char *word);
int get_unique_letters(char *word, int wordlen);
void print_word(char *word, int wordlen, char *guessed, int guessedlen);
void unload_words(node *rootNode);

int main()
{
    // Root node of linked list of words
    node *wordlist = NULL;
    int wordnum = load_words(&wordlist);
    if (!(wordnum > 0))
    {
        printf("0 words were loaded. Stopping execution...\n");
        exit(1);
    }

    for (;;)
    {
        char *word = get_random_word(wordlist, wordnum);
        int wordlen = word_to_upper(word);
        if (!play(word, wordlen)) break;
    }

    unload_words(wordlist);
    return 0;
}

// Starts round and returns true if user wants to play again
bool play(char *word, int wordlen)
{
    printf("The word has %i letters\n", wordlen);

    // Get number of tries from user
    char input[3];
    bool isValid;
    do
    {
        isValid = true;
        size_t inplen = 0;

        printf("How many tries do you want?\n> ");
        get_input(input, sizeof(input), &inplen);

        for (int i = 0; i < inplen; i++)
        {
            if (isdigit(input[i]) == 0) isValid = false;
        }
    }
    while (!isValid);

    int tries = strtol(input, (char **)NULL, 10);
    if (errno == ERANGE)
    {
        printf("error converting input to integer\n");
        exit(1);
    }

    // Number of guessed letters
    int guessed = 0;
    int toGuess = get_unique_letters(word, wordlen);

    // List of used letters
    char usedLetters[ALPHABET + 1] = "\0";
    // Index to add next used letter
    int usedLettersIndex = 0;

    char letter;

    while (guessed < toGuess && tries > 0)
    {
        printf("\nYou have %i tries left and have used the letters: ", tries);
        for (int i = 0; i < usedLettersIndex; i++)
        {
            printf("%c", usedLetters[i]);
            if (usedLetters[i + 1] != '\0') printf(", ");
        }
        printf("\n");
        print_word(word, wordlen, usedLetters, usedLettersIndex);
        printf("\n");

        // Prompt user for letter
        do
        {
            isValid = false;
            printf("Letter: ");
            letter = toupper(get_char());

            if (isalpha(letter)) isValid = true;

            // Check if letter has aready been used
            for (int i = 0; usedLetters[i] != '\0'; i++)
            {
                if (usedLetters[i] == letter)
                {
                    printf("You have already used that letter.\n");
                    isValid = false;
                    break;
                }
            }
        }
        while (!isValid);

        // Add letters to used letters
        usedLetters[usedLettersIndex] = letter;
        usedLettersIndex++;

        // Check if letter is in word
        bool inWord = false;
        for (int i = 0; i < wordlen; i++)
        {
            if (letter == word[i])
            {
                inWord = true;
                guessed++;
                break;
            }
        }
        if (!inWord) tries--;
    }

    if (tries > 0)
        printf("\n--- YOU WON! ---\n");
    else
        printf("\n--- YOU LOSE! ---\n");
    printf("The word was %s\n", word);

    // Prompt user to play again
    printf("\nDo you want to play again? (Y/n)\n> ");
    char answer = get_char();

    printf("\n");
    if (toupper(answer) == 'Y')
        return true;
    return false;
}

// Get input from user and put its length in *INPLEN if INPLEN is not NULL
char *get_input(char *buff, size_t size, size_t *inplen)
{
    buff[0] = '\0';
    size_t len = 0;

    if (fgets(buff, size, stdin) != NULL)
    {
        len = strlen(buff);
        char *endp = len == 0 ? buff : &buff[len - 1];

        if (*endp != '\n')
        {
            // Clear keyboard buffer
            int discard;
            while ((discard = getchar()) != '\n' && discard != EOF) { }
        }
        else
        {
            // Remove newline from buff
            *endp = '\0';
            len--;
        }
        if (inplen != NULL) *inplen = len;
    }
    return buff;
}

// Uses getchar() and clears buffer
char get_char()
{
    char c = getchar();

    int discard;
    while ((discard = getchar()) != '\n' && discard != EOF);

    return c;
}

// Returns number of different letters in word
int get_unique_letters(char *word, int wordlen)
{
    char uniqueLetters[wordlen + 1];
    // Index to add new letter
    int index = 0;
    // Remove garbage
    for (int i = 0, l = wordlen + 1; i < l; i++) uniqueLetters[i] = '\0';

    char letter;
    // For each letter in word
    for (int i = 0; (letter = word[i]) != '\0'; i++)
    {
        bool isNew = true;
        char uniqueLetter;
        // Check if `letter` is in `uniqueLetters`
        for (int j = 0; (uniqueLetter = uniqueLetters[j]) != '\0'; j++)
        {
            if (uniqueLetter == letter)
            {
                isNew = false;
                break;
            }
        }
        if (isNew)
        {
            uniqueLetters[index] = letter;
            index++;
        }
    }
    return index;
}

// Prints word, changing each letter to '-' if user hasn't guessed it yet
void print_word(char *word, int wordlen, char *guessed, int guessedlen)
{
    printf("WORD: ");
    // For each letter in word
    for (int i = 0; i < wordlen; i++)
    {
        // Check if user has already guessed letter
        bool hasGuessed = false;
        char *letter;
        for (int j = 0; j < guessedlen; j++)
        {
            if (*(letter = &word[i]) == guessed[j])
            {
                hasGuessed = true;
                break;
            }
        }
        if (hasGuessed)
            printf("%c ", *letter);
        else
            printf("- ");
    }
}

// Loads words into memory and returns number of words
int load_words(node **rootNode)
{
    int wordCount = 0;
    if (*rootNode == NULL)
    {
        FILE *f = fopen(FILENAME, "r");
        if (f != NULL)
        {
            // Current word
            char cword[WORDLEN + 1];

            node *lastNode = NULL;
            node *tmp = NULL;
            while (fgets(cword, sizeof(cword), f) != NULL)
            {
                // Remove newline from cword
                cword[strcspn(cword, "\n")] = '\0';

                // Create node
                tmp = malloc(sizeof(node));
                strcpy(tmp -> word, cword);
                tmp -> next = NULL;

                // Add node to list
                if (*rootNode == NULL)
                {
                    *rootNode = tmp;
                    lastNode = *rootNode;
                }
                else
                {
                    lastNode -> next = tmp;
                    lastNode = lastNode -> next;
                }
                wordCount++;
            }
            fclose(f);
        }
    }
    return wordCount;
}

// Returns random word
char *get_random_word(node *rootNode, int wordCount)
{
    srand(time(NULL));
    int wordIndex = rand() % wordCount;

    node *n = rootNode;
    for (int i = 0; i < wordIndex; i++) n = n -> next;

    return n -> word;
}

// Makes word uppercase and returns length of word
int word_to_upper(char *word)
{
    int len = 0;
    for (char *c = &word[0]; *c != '\0'; c = &word[len])
    {
        *c = toupper(*c);
        len++;
    }
    return len;
}

// Unload words from memory
void unload_words(node *rootNode)
{
    if (rootNode != NULL)
    {
        node *current = rootNode;
        node *next = NULL;
        while (current != NULL)
        {
            next = current -> next;
            free(current);
            current = next;
        }
    }
}
