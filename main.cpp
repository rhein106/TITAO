#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

struct TitaoGame
{
    int board[3][3];
    unsigned int max_rounds;
    const char *leaderboard_file_path;
} titao_game = {
    {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
    },
    5,
    "leaderboard.txt",
};

typedef struct Player
{
    char name[101];
    long int win_count, lost_count, draw_count;
} player_t;

player_t player_one = {0, 0, 0};
player_t player_two = {0, 0, 0};

void menu_play();
void menu_leaderboard();
void menu_exit();

char parse_xo(int xo);
int check_winner();

void board_draw();
void board_reset();
int board_position_is_empty(int x, int y);
int board_insert_at(int row, int column, int value);
int board_is_full();

void reset_players();

unsigned int leaderboard_size(FILE *fp);
void sort_leaderboard(player_t *leaderboard_players, int left, int right, int option, int descending);
void merge_leaderboard(player_t *leaderboard_players, int left, int mid, int right, int option, int descending);
int search_leaderboard(player_t *leaderboard_players, int n, char name[101]);
void update_leaderboard();
void remove_player_from_leaderboard(char name[101]);

void sleep_ms(int milliseconds);

int main()
{
    short int option;
    do
    {
        system("cls||clear");
        puts("\t\t\t*~*~*~*~*~* Titao *~*~*~*~*~*~");
        puts("\t\t\t~         Main Menu          *");
        puts("\t\t\t*  1. Play                   ~");
        puts("\t\t\t~  2. Leaderboard            *");
        puts("\t\t\t*  3. Exit                   ~");
        puts("\t\t\t~                            *");
        puts("\t\t\t*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~\n\n\n");
        printf("Select menu: ");
        scanf("%hd", &option);
        puts("");
        switch (option)
        {
        case 1:
            menu_play();
            break;
        case 2:
            menu_leaderboard();
            break;
        case 3:
            menu_exit();
            break;
        }
    } while (option != 3);
    return 0;
}

void menu_play()
{
    int rounds;
    system("cls||clear");

input_player_one:
    printf("Input nama player 1 (X): ");
    scanf("%s", player_one.name);

    if (strlen(player_one.name) > 100)
    {
        puts("Nama player 1 terlalu panjang (max: 100)");
        goto input_player_one;
    }

input_player_two:
    printf("Input nama player 2 (O): ");
    scanf("%s", player_two.name);

    if (strlen(player_two.name) > 100)
    {
        puts("Nama player 2 terlalu panjang (max: 100)");
        goto input_player_two;
    }

input_round:
    printf("Jumlah Ronde (max 5): ");
    scanf("%d", &rounds);

    if (rounds > titao_game.max_rounds)
    {
        printf("! Maksimal ronde hanya %d !\n", titao_game.max_rounds);
        goto input_round;
    }

    int winner = 0;
    int player_one_round_score = 0;
    int player_two_round_score = 0;
    for (int current_round = 1; current_round <= rounds; current_round++)
    {
        int current_move = 1;
        printf("Round %d\n", current_round);
        do
        {
        reset_round:
            winner = check_winner();

            board_draw();
            if (winner != 0)
            {
                char current_player[255];
                if (winner == -1)
                {
                    player_one_round_score++;
                    strcpy(current_player, player_one.name);
                }
                else if (winner == 1)
                {
                    player_two_round_score++;
                    strcpy(current_player, player_two.name);
                }
                else if (winner == 2)
                {
                    // If the board is full and there is no winners, then it's a draw
                    player_one_round_score++;
                    player_two_round_score++;

                    system("cls||clear");
                    puts("==== Ronde Berakhir dengan Draw ====");
                    printf("%s: %d\n", player_one.name, player_one_round_score);
                    printf("%s: %d\n", player_two.name, player_two_round_score);
                    break;
                }

                system("cls||clear");
                printf("==== %s Menang Ronde %d ====\n\n", current_player, current_round);
                printf("%s: %d\n", player_one.name, player_one_round_score);
                printf("%s: %d\n", player_two.name, player_two_round_score);
                break;
            }

            int position;
            printf("Input posisi (1-9): ");
            scanf("%d", &position);

            if (position == 1 || position == 2 || position == 3)
            {
                if (board_insert_at(0, position - 1, current_move % 2 != 0 ? -1 : 1) == -1)
                {
                    puts("Posisi sudah terisi");
                    goto reset_round;
                }
            }
            else if (position == 4 || position == 5 || position == 6)
            {
                if (board_insert_at(1, position - 4, current_move % 2 != 0 ? -1 : 1) == -1)
                {
                    puts("Posisi sudah terisi");
                    goto reset_round;
                }
            }
            else if (position == 7 || position == 8 || position == 9)
            {
                if (board_insert_at(2, position - 7, current_move % 2 != 0 ? -1 : 1) == -1)
                {
                    puts("Posisi sudah terisi");
                    goto reset_round;
                }
            }
            else
            {
                puts("Invalid Position (1-9)");
            }

            if (current_move % 2 != 0)
            {
                current_move++;
            }
            else
            {
                current_move--;
            }
        } while (winner == 0);

        board_reset();
        if (player_one_round_score > player_two_round_score)
        {
            player_one.win_count++;
            player_two.lost_count++;
        }
        else if (player_one_round_score < player_two_round_score)
        {
            player_two.win_count++;
            player_one.lost_count++;
        }
        else if (player_one_round_score == player_two_round_score)
        {
            player_one.draw_count++;
            player_two.draw_count++;
        }
    }

    // Save to leaderboards
    update_leaderboard();

    if (player_one_round_score > player_two_round_score)
    {
        printf("%s wins the game!\n", player_one.name);
    }

    if (player_one_round_score < player_two_round_score)
    {
        printf("%s wins the game!\n", player_two.name);
    }

    if (player_one_round_score == player_two_round_score)
    {
        printf("The game is a draw!\n");
    }

    reset_players();
    printf("\nGoing back to the main menu... (4s)\n");
    sleep_ms(4000);
}

void menu_leaderboard()
{

leaderboard:
    FILE *leaderboards_fp = fopen(titao_game.leaderboard_file_path, "r");
    if (leaderboards_fp == NULL)
    {
        puts("Leaderboard is still empty");
        return;
    }

    int i = 0;
    int size = leaderboard_size(leaderboards_fp);
    rewind(leaderboards_fp);

    player_t *leaderboard_players = (player_t *)malloc(sizeof(player_t) * size);

    int total_wins = 0, total_lost = 0, total_draw = 0;
    double average_wins = 0, average_lost = 0, average_draw = 0;

    while (fscanf(leaderboards_fp, "%[^#]#%ld#%ld#%ld\n",
                  leaderboard_players[i].name, &leaderboard_players[i].win_count,
                  &leaderboard_players[i].lost_count, &leaderboard_players[i].draw_count) != EOF)
    {
        total_wins += leaderboard_players[i].win_count;
        total_lost += leaderboard_players[i].lost_count;
        total_draw += leaderboard_players[i].draw_count;
        i++;
    }

    average_wins = (double)total_wins / (double)size;
    average_lost = (double)total_lost / (double)size;
    average_draw = (double)total_draw / (double)size;

    short int option, descending = 0;

    do
    {
        system("cls||clear");
        puts("\t\t\t*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~");
        puts("\t\t\t~         Leaderboard        *");
        puts("\t\t\t*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~");

        printf("Total wins: %d\n", total_wins);
        printf("Total lost: %d\n", total_lost);
        printf("Total draw: %d\n", total_draw);
        printf("Average wins: %.2f\n", average_wins);
        printf("Average lost: %.2f\n", average_lost);
        printf("Average draw: %.2f\n\n\n", average_draw);

        printf("\t%s\t\t\t%s\t\t\t%s\t\t\t%s\n", "Name", "Wins", "Losses", "Draws");
        printf("\t%s\t\t\t%s\t\t\t%s\t\t\t%s\n", "----", "----", "------", "------");

        for (int i = 0; i < size; i++)
        {
            printf("%d)\t%s\t\t\t%ld\t\t\t%ld\t\t\t%ld\n", i + 1,
                   leaderboard_players[i].name, leaderboard_players[i].win_count,
                   leaderboard_players[i].lost_count, leaderboard_players[i].draw_count);
        }

        puts("\n\n1. Sort by name");
        puts("2. Sort by wins");
        puts("3. Sort by losses");
        puts("4. Sort by draws");
        puts("5. Search by name");
        puts("6. Remove player");
        puts("7. Back");
        printf("\n\nSelect menu: ");
        scanf("%hd", &option);

        switch (option)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        {
            puts("\n\n1. Ascending");
            puts("2. Descending");
            puts("3. Back");
            printf("\n\n Select menu: ");
            scanf("%hd", &descending);
            if (descending == 3)
            {
                goto leaderboard;
            }
            sort_leaderboard(leaderboard_players, 0, size - 1, option, descending - 1);
            break;
        }
        case 5:
        {
        leaderboard_search:
            printf("\n\nSearch: ");
            char search[101];
            scanf("%s", search);
            // Sort by name first
            sort_leaderboard(leaderboard_players, 0, size - 1, 1, 0);
            int idx_of_player = search_leaderboard(leaderboard_players, size, search);
            if (idx_of_player == -1)
            {
                puts("\n\nPlayer not found.");
                goto leaderboard_search;
            }
            else
            {

                printf("\t%s\t\t\t%s\t\t\t%s\t\t\t%s\n", "Name", "Wins", "Losses", "Draws");
                printf("\t%s\t\t\t%s\t\t\t%s\t\t\t%s\n", "----", "----", "------", "------");
                printf("%d)\t%s\t\t\t%ld\t\t\t%ld\t\t\t%ld\n\n\n", idx_of_player + 1,
                       leaderboard_players[idx_of_player].name, leaderboard_players[idx_of_player].win_count,
                       leaderboard_players[idx_of_player].lost_count, leaderboard_players[idx_of_player].draw_count);

                puts("1. Search again");
                puts("2. Back");
                printf("\n\nSelect menu: ");
                scanf("%hd", &option);
                if (option == 1)
                {
                    goto leaderboard_search;
                }
                else if (option == 2)
                {
                    goto leaderboard;
                }
            }
            break;
        }
        case 6:
        {
        leaderboard_remove:
            printf("\n\nRemove: ");
            char remove[101];
            scanf("%s", remove);
            // Sort by name first
            sort_leaderboard(leaderboard_players, 0, size - 1, 1, 0);
            int idx_to_remove = search_leaderboard(leaderboard_players, size, remove);
            if (idx_to_remove == -1)
            {
                puts("\n\nPlayer not found.");
                goto leaderboard_remove;
            }
            else
            {
                puts("\n\nAre you sure you want to remove this player? (y/n)");
                char confirm;
                scanf("\n%c", &confirm);
                if (confirm == 'y')
                {
                    remove_player_from_leaderboard(remove);
                    puts("\n\nPlayer removed.");
                }
                else
                {
                    puts("\n\nPlayer not removed.");
                }
                goto leaderboard;
            }
            break;
        }
        }
    } while (option != 7);
}

void menu_exit()
{
    puts("\t\t\t*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*");
    puts("\t\t\t* Terima Kasih Sudah Bermain! ~");
    puts("\t\t\t*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*");
}

void board_draw()
{
    printf("\n\n\tSelamat Bermain!\n\n");

    printf("%s (X)  -  %s (O)\n\n\n", player_one.name, player_two.name);

    printf("___________________\n");

    printf("|     |     |     |\n");
    printf("|  %c  |  %c  |  %c  |\n", parse_xo(titao_game.board[0][0]), parse_xo(titao_game.board[0][1]), parse_xo(titao_game.board[0][2]));

    printf("|_____|_____|_____|\n");
    printf("|     |     |     |\n");

    printf("|  %c  |  %c  |  %c  |\n", parse_xo(titao_game.board[1][0]), parse_xo(titao_game.board[1][1]), parse_xo(titao_game.board[1][2]));

    printf("|_____|_____|_____|\n");
    printf("|     |     |     |\n");

    printf("|  %c  |  %c  |  %c  |\n", parse_xo(titao_game.board[2][0]), parse_xo(titao_game.board[2][1]), parse_xo(titao_game.board[2][2]));

    printf("|_____|_____|_____|\n\n");
}

void board_reset()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            titao_game.board[i][j] = 0;
        }
    }
}

char parse_xo(int xo)
{
    if (xo == -1)
        return 'X';
    if (xo == 1)
        return 'O';
    return ' ';
}

int check_winner()
{
    for (int i = 0; i < 3; i++) // Check row
        if (titao_game.board[i][0] == titao_game.board[i][1] &&
            titao_game.board[i][0] == titao_game.board[i][2])
            return titao_game.board[i][0];

    for (int i = 0; i < 3; i++) // Check column
        if (titao_game.board[0][i] == titao_game.board[1][i] &&
            titao_game.board[0][i] == titao_game.board[2][i])
            return titao_game.board[0][i];

    // Check diagonal
    if (titao_game.board[0][0] == titao_game.board[1][1] &&
        titao_game.board[1][1] == titao_game.board[2][2])
        return titao_game.board[0][0];

    // Check diagonal
    if (titao_game.board[0][2] == titao_game.board[1][1] &&
        titao_game.board[1][1] == titao_game.board[2][0])
        return titao_game.board[0][2];

    // Check if board is full and no winner
    if (board_is_full() == 1)
    {
        return 2;
    }
    return 0;
}

int board_position_is_empty(int row, int column)
{
    return titao_game.board[row][column] == 0;
}

int board_insert_at(int row, int column, int value)
{
    if (!board_position_is_empty(row, column))
    {
        return -1;
    }

    titao_game.board[row][column] = value;
    return 1;
}

int board_is_full()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (titao_game.board[i][j] == 0)
                return -1;
        }
    }
    return 1;
}

void reset_players()
{
    memset(&player_one, 0, sizeof(player_t));
    memset(&player_two, 0, sizeof(player_t));
}

// Counts the number of lines in a file
unsigned int leaderboard_size(FILE *fp)
{
    unsigned int lines = 0;

    // Get amount of lines in a file https://www.geeksforgeeks.org/c-program-count-number-lines-file/
    for (char tmp_c = getc(fp); tmp_c != EOF; tmp_c = getc(fp))
    {
        // Increment lines if the current character is newline
        if (tmp_c == '\n')
        {
            lines++;
        }
    }

    return lines;
}

// Option 1 = sort by name
// Option 2 = sort by wins
// Option 3 = sort by losses
// Option 4 = sort by draws
void sort_leaderboard(player_t *leaderboard_players, int left, int right, int option, int descending)
{
    if (left >= right)
    {
        return;
    }

    int mid = (left + right) / 2;

    sort_leaderboard(leaderboard_players, left, mid, option, descending);
    sort_leaderboard(leaderboard_players, mid + 1, right, option, descending);

    merge_leaderboard(leaderboard_players, left, mid, right, option, descending);
}

void merge_leaderboard(player_t *leaderboard_players, int left, int mid, int right, int option, int descending)
{
    int left_partition_size = mid - left + 1;
    int right_partition_size = right - mid;

    player_t left_partition[left_partition_size];
    player_t right_partition[right_partition_size];

    for (int i = 0; i < left_partition_size; i++)
    {
        left_partition[i] = leaderboard_players[left + i];
    }

    for (int i = 0; i < right_partition_size; i++)
    {
        right_partition[i] = leaderboard_players[mid + 1 + i];
    }

    int idx = left;
    int idx_l = 0;
    int idx_r = 0;
    while (idx_l < left_partition_size && idx_r < right_partition_size)
    {
        if (option == 1 && descending)
        {
            if (strcmp(left_partition[idx_l].name, right_partition[idx_r].name) > 0)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 1 && !descending)
        {
            if (strcmp(left_partition[idx_l].name, right_partition[idx_r].name) < 0)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 2 && descending)
        {
            if (left_partition[idx_l].win_count > right_partition[idx_r].win_count)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 2 && !descending)
        {
            if (left_partition[idx_l].win_count < right_partition[idx_r].win_count)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 3 && descending)
        {
            if (left_partition[idx_l].lost_count > right_partition[idx_r].lost_count)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 3 && !descending)
        {
            if (left_partition[idx_l].lost_count < right_partition[idx_r].lost_count)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 4 && descending)
        {
            if (left_partition[idx_l].draw_count > right_partition[idx_r].draw_count)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }

        if (option == 4 && !descending)
        {
            if (left_partition[idx_l].draw_count < right_partition[idx_r].draw_count)
            {
                leaderboard_players[idx] = left_partition[idx_l];
                idx++;
                idx_l++;
            }
            else
            {
                leaderboard_players[idx] = right_partition[idx_r];
                idx++;
                idx_r++;
            }
        }
    }

    while (idx_l < left_partition_size)
    {
        leaderboard_players[idx] = left_partition[idx_l];
        idx++;
        idx_l++;
    }

    while (idx_r < right_partition_size)
    {
        leaderboard_players[idx] = right_partition[idx_r];
        idx++;
        idx_r++;
    }
}

int search_leaderboard(player_t *leaderboard_players, int n, char name[101])
{
    int min = 0, max = n - 1;
    while (min <= max)
    {
        int mid = (min + max) / 2;

        if (strcmp(name, leaderboard_players[mid].name) < 0)
        {
            max = mid - 1;
        }

        else if (strcmp(name, leaderboard_players[mid].name) > 0)
        {
            min = mid + 1;
        }
        else
        {
            return mid;
        }
    }
    return -1;
}

void update_leaderboard()
{
    FILE *leaderboards_fp = fopen(titao_game.leaderboard_file_path, "a+");

    // If the file is empty then we can assume that it is the first time the user is playing
    // and we can write the player's name and score to the file
    if (fgetc(leaderboards_fp) == EOF)
    {
        puts("EMPTY");
        fprintf(leaderboards_fp, "%s#%ld#%ld#%ld\n", player_one.name, player_one.win_count, player_one.lost_count, player_one.draw_count);
        fprintf(leaderboards_fp, "%s#%ld#%ld#%ld\n", player_two.name, player_two.win_count, player_two.lost_count, player_two.draw_count);
        fclose(leaderboards_fp);
        return;
    }

    unsigned int lines = leaderboard_size(leaderboards_fp);

    player_t *leaderboard_players = (player_t *)malloc(lines * sizeof(player_t));
    if (leaderboard_players == NULL)
    {
        puts("Error allocating memory");
        return;
    }

    // Rewind the file pointer to the beginning of the file
    rewind(leaderboards_fp);

    // Set all values to 0
    memset(leaderboard_players, 0, sizeof(player_t) * lines);

    int i = 0;
    while (fscanf(leaderboards_fp, "%[^#]#%ld#%ld#%ld\n",
                  leaderboard_players[i].name, &leaderboard_players[i].win_count,
                  &leaderboard_players[i].lost_count, &leaderboard_players[i].draw_count) != EOF)
    {
        i++;
    }

    int found_player_one = -1;
    int found_player_two = -1;
    for (int i = 0; i < lines; i++)
    {
        if (strcmp(player_one.name, leaderboard_players[i].name) == 0)
        {
            leaderboard_players[i].win_count += player_one.win_count;
            leaderboard_players[i].lost_count += player_one.lost_count;
            leaderboard_players[i].draw_count += player_one.draw_count;
            found_player_one = 1;
        }
        else if (strcmp(player_two.name, leaderboard_players[i].name) == 0)
        {
            leaderboard_players[i].win_count += player_two.win_count;
            leaderboard_players[i].lost_count += player_two.lost_count;
            leaderboard_players[i].draw_count += player_two.draw_count;
            found_player_two = 1;
        }
    }

    if (found_player_one == -1)
    {
        lines++;
    }
    if (found_player_two == -1)
    {
        lines++;
    }

    // Increase the size of the array by the amount of new lines
    leaderboard_players = (player_t *)realloc(leaderboard_players, sizeof(player_t) * lines);

    if (found_player_one == -1 && found_player_two == -1)
    {
        player_t tmp_player;
        strcpy(tmp_player.name, player_one.name);
        tmp_player.win_count = player_one.win_count;
        tmp_player.lost_count = player_one.lost_count;
        tmp_player.draw_count = player_one.draw_count;
        leaderboard_players[lines - 1] = tmp_player;

        strcpy(tmp_player.name, player_two.name);
        tmp_player.win_count = player_two.win_count;
        tmp_player.lost_count = player_two.lost_count;
        tmp_player.draw_count = player_two.draw_count;
        leaderboard_players[lines - 2] = tmp_player;
    }
    else if (found_player_one == -1)
    {
        player_t tmp_player;
        strcpy(tmp_player.name, player_one.name);
        tmp_player.win_count = player_one.win_count;
        tmp_player.lost_count = player_one.lost_count;
        tmp_player.draw_count = player_one.draw_count;
        leaderboard_players[lines - 1] = tmp_player;
    }
    else if (found_player_two == -1)
    {
        player_t tmp_player;
        strcpy(tmp_player.name, player_two.name);
        tmp_player.win_count = player_two.win_count;
        tmp_player.lost_count = player_two.lost_count;
        tmp_player.draw_count = player_two.draw_count;
        leaderboard_players[lines - 1] = tmp_player;
    }

    // Sort the array by wins and descending by default
    sort_leaderboard(leaderboard_players, 0, lines - 1, 2, 1);

    // Rewind the file pointer to the beginning of the file
    rewind(leaderboards_fp);

    // Truncate the file and write the new data
    freopen(titao_game.leaderboard_file_path, "w", leaderboards_fp);

    // Write the new leaderboard to the file
    for (int i = 0; i < lines; i++)
    {
        fprintf(leaderboards_fp, "%s#%ld#%ld#%ld\n",
                leaderboard_players[i].name, leaderboard_players[i].win_count,
                leaderboard_players[i].lost_count, leaderboard_players[i].draw_count);
    }

    fclose(leaderboards_fp);
    free(leaderboard_players);
}

void remove_player_from_leaderboard(char name[101])
{
    FILE *leaderboards_fp = fopen(titao_game.leaderboard_file_path, "a+");
    if (leaderboards_fp == NULL)
    {
        puts("Error opening file");
        return;
    }

    unsigned int lines = leaderboard_size(leaderboards_fp);

    player_t *leaderboard_players = (player_t *)malloc(lines * sizeof(player_t));
    if (leaderboard_players == NULL)
    {
        puts("Error allocating memory");
        return;
    }

    // Rewind the file pointer to the beginning of the file
    rewind(leaderboards_fp);

    // Set all values to 0
    memset(leaderboard_players, 0, sizeof(player_t) * lines);

    int i = 0;
    while (fscanf(leaderboards_fp, "%[^#]#%ld#%ld#%ld\n",
                  leaderboard_players[i].name, &leaderboard_players[i].win_count,
                  &leaderboard_players[i].lost_count, &leaderboard_players[i].draw_count) != EOF)
    {
        i++;
    }

    sort_leaderboard(leaderboard_players, 0, lines - 1, 1, 0);

    int idx = search_leaderboard(leaderboard_players, lines, name);

    if (idx == -1)
    {
        puts("Player not found");
        return;
    }

    // Remove the player in the array
    for (int i = idx; i < lines - 1; i++)
    {
        leaderboard_players[i] = leaderboard_players[i + 1];
    }

    // Decrease the size of the array by 1
    leaderboard_players = (player_t *)realloc(leaderboard_players, sizeof(player_t) * (lines - 1));

    // Rewind the file pointer to the beginning of the file
    rewind(leaderboards_fp);

    // Truncate the file and write the new data
    freopen(titao_game.leaderboard_file_path, "w", leaderboards_fp);

    // Write the new leaderboard to the file
    for (int i = 0; i < lines - 1; i++)
    {
        fprintf(leaderboards_fp, "%s#%ld#%ld#%ld\n",
                leaderboard_players[i].name, leaderboard_players[i].win_count,
                leaderboard_players[i].lost_count, leaderboard_players[i].draw_count);
    }

    fclose(leaderboards_fp);
    free(leaderboard_players);
}

void sleep_ms(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}
