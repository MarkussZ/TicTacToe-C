//Minmax implementation based on :https://gist.github.com/MatthewSteel/3158579#file-ttt-c-L8
#include <game_strings.h>
#include <tic_tac_toe.h>

static struct game_settings *init_game_settings()
{
	struct game_settings *result = malloc(sizeof(struct game_settings));
	result->AI_name = malloc(sizeof(char) * BUFF_SIZE);
	result->language = malloc(sizeof(char) * BUFF_SIZE);
	result->p1_name = malloc(sizeof(char) * BUFF_SIZE);
	result->p2_name = malloc(sizeof(char) * BUFF_SIZE);
	result->game_strings = NULL;
	result->board_size = 0;
	memset(result->language,0,BUFF_SIZE);
	memset(result->p1_name,0,BUFF_SIZE);
	memset(result->p2_name,0,BUFF_SIZE);
	memset(result->AI_name,0,BUFF_SIZE);
}

int main(int argc,char *argv[])
{
	char *menu_choice = malloc(sizeof(char)*3);

	struct game_settings *settings = parse_cmd_args(argc,argv);

	printf("TicTacToe v%s\n",TTT_VERSION);
	do{
		printf("%s\n",settings->game_strings[0]);
		do{
			fgets(menu_choice,3,stdin);
		}while(*menu_choice != '1' && *menu_choice != '2');
		game((*menu_choice)-'0',settings);

		printf("%s\n", settings->game_strings[8]);
		fgets(menu_choice,3,stdin);
	} while(*menu_choice == 'Y' || *menu_choice == 'y');

	free(settings);
	free(menu_choice);

	return 0;
}

void game(int player_choice,const struct game_settings *settings)
{
	char buffer[5], *p2_name, *p1_name;
	char **board = malloc(sizeof(char*)*settings->board_size);
	int seed,response = 0;
	char current_player = 0;//default state. 1 for 'x', -1 for 'o'
	int turn_count;
	int input = 0;
	time_t tt;
	int game_state = 0;

	p1_name = settings->p1_name;
	p2_name = (player_choice == 1 ? settings->AI_name : settings->p2_name);

	seed = time(&tt);/*Time passed since 01.01.1970 is used as randomizer seed*/
	srand(seed);
	for(int i = 0;i < settings->board_size; i++){
		board[i] = malloc(sizeof(char)*settings->board_size);
		memset(board[i],0,settings->board_size);
	}

	printf("%s\n",settings->game_strings[1]);
	print_board(board,settings->board_size,settings->p1_sign,settings->p2_sign,
			settings->ff_sign);

	current_player = (seed % 2 == 0? 1:-1);
	printf("%s%s\n", ((current_player==-1) ? p1_name : p2_name), settings->game_strings[2]);

	for(turn_count = 0;turn_count < settings->board_size * settings->board_size ;turn_count++)/*Main gameplay loop*/
	{
		if (current_player == 1 && player_choice == 1){
			input = ai_analyze_board_state(board,settings->board_size,current_player);
		}else{
			printf("%s%s\n", ((current_player == -1) ? p1_name : p2_name),settings->game_strings[3]);
			fgets(buffer, sizeof(buffer), stdin);
			input = strtol(buffer,NULL,10);
			memset(buffer,0,5);
		}
		response = move(input, current_player,board,settings->board_size);
		while(response == -1) {
			printf("%s\n", settings->game_strings[4]);
			fgets(buffer, sizeof(buffer), stdin);
			input = strtol(buffer,NULL,10);
			response = move(input, current_player,board,settings->board_size);
		}
		printf("%s %s %d!\n",(current_player == -1 ? p1_name : p2_name),settings->game_strings[5],input);
		print_board(board,settings->board_size,settings->p1_sign,settings->p2_sign,
				settings->ff_sign);
		game_state = win_check(board,settings->board_size,current_player);
		current_player = current_player*(-1);//Turn is about to end, so turn bool's value is reversed
		if(  (game_state == 1 || game_state == -1)){
			printf("%s %s\n", (game_state == -1) ? p1_name : p2_name,settings->game_strings[6]);
			break;
		}
	}

	if(game_state == 0 &&win_check(board,settings->board_size,current_player) == 0) {
		printf("%s\n",settings->game_strings[7]);
	}
	for(int i = 0; i < settings->board_size; i++){
		free(board[i]);
	}
	free(board);
}

int move(char field, int current_player,char **board,int board_size)
{
	if(field <= 0 || field > board_size * board_size){
		return -1;
	}

	field--; //Standarize input from 1-based fields to 0-based
	int y_offset = field / board_size;
	int x_offset = field % board_size;
	if (board[y_offset][x_offset] == 0) {
		board[y_offset][x_offset] = current_player;
		return 0;
	}
	
	return -1;
}

void print_board(char **board, int board_size,char p1_sign,char p2_sign,char ff_sign)
{
	for(int i = 0; i < board_size ; i++) {
		for(int j = 0 ; j < board_size ; j++) {
			switch(board[i][j]){
				case -1:
					printf("%c",p1_sign);
					break;
				case 1:
					printf("%c",p2_sign);
					break;
				default:
					printf("%c",ff_sign);
					break;
			}
			if(j == board_size - 1) {
				printf("\n");
			} else {
				printf("|");
			}
		}
	}
	printf("\n");
}

unsigned ***wins_generator(int board_size){
	int wins_count = (board_size == 1 ? 1 : 2*board_size + 2);
	unsigned ***wins = malloc(sizeof(unsigned**) * wins_count);
	for(int i=0;i<wins_count;i++){
		wins[i] = malloc(sizeof(unsigned*) * board_size);
		for(int j=0;j<board_size;j++){
			wins[i][j] = malloc(sizeof(unsigned)*2);
		}
	}
	int i,t=0;
	for(i = 0; i < board_size; i++){
		for(int j = 0; j <board_size;j++){
			wins[i][j][0] = j;
			wins[i][j][1] = t;
			wins[i+board_size][j][0] = t;
			wins[i+board_size][j][1] = j;
		}
		t++;
	}
	t = 0;
	for(; i < 2*board_size; i++){
		for(int j = 0; j <board_size;j++){
			wins[i][j][0] = t;
			wins[i][j][1] = j;
		}
		t++;
	}
	for(int j=0;j<board_size;j++){
		wins[i][j][0]=j;
		wins[i][j][1]=j;
	}
	i++;
	for(int j=0;j<board_size;j++){
		wins[i][j][0]=board_size-j-1;
		wins[i][j][1]=j;
	}
	return wins;
}

void printf_wins(unsigned ***wins,int board_size)
{
	/* For debugging purposes only */
	for(int i = 0;i<2*board_size+2;i++){
		for(int j=0;j<board_size;j++){
			printf("%u,%u|",wins[i][j][0],wins[i][j][1]);
		}
		printf("\n");
	}
}

int free_fields_count(char ** board,int board_size)
{
	int count = 0;
	for(int i = 0;i<board_size;i++){
		for(int j = 0;j<board_size; j++){
			if(board[i][j] == 0){
				count++;
			}
		}
	}
	return count;
}

int win_check(char **board,int board_size,int current_player)
{
	int i,p;
	static unsigned ***wins = NULL;
	static long long calls = 0;
	if(wins == NULL){
		wins = wins_generator(board_size);
	}
	for(i = 0;i<2*board_size+2;i++){
		for(p=0;p<board_size;p++){
			if(board[wins[i][p][0]][wins[i][p][1]] == current_player && p == board_size - 1) {
				return current_player;
			} else if(board[wins[i][p][0]][wins[i][p][1]] != current_player) {
				break;
			}
		}
	}
	return 0;
}

int ai_analyze_board_state(char **board,int board_size,int current_player)
{
	int move_i = -1, move_j = -1;
	int score = -2;
	const int depth_b_size_ratio = board_size+4;
	/* 5/3 is a magic number, but that's what
	 * I found working well for each kind of input
	 */
	for(int i=0;i<board_size;++i){
		for(int j=0;j<board_size;++j){
			if(board[i][j] == 0) {
				board[i][j] = current_player;
				int tempScore = minimax(-current_player,board,board_size,
						depth_b_size_ratio,-100000,100000,current_player);
				board[i][j] = 0;
				if(tempScore >= score){
					score = tempScore;
					move_i = i;
					move_j = j;
				}
			}
		}
	}
	return board_size * move_i + move_j + 1;
}

int minimax(int player,char **board,int board_size,int depth,int alpha,int beta,int init_player)
{
	int winner = win_check(board,board_size,player);
	if(depth == 0 || (winner == 0 && free_fields_count(board,board_size) == 0) ){
		return 0;
	}
	if (winner != 0) {
		return (board_size*board_size-depth)*(winner==init_player?1:-1);
	}
	int i,j;
	int move = -1;
	for(i=0;i<board_size;++i){
		for(j=0;j<board_size;++j){
			if( board[i][j] == 0){
				board[i][j] = player;
				int MyScore = minimax(-player,board,board_size,depth-1,alpha,beta,init_player);
				board[i][j] = 0;
				if(player == -init_player){
					beta = (MyScore > beta? beta: MyScore); 
				} else if(player == init_player){
					alpha = (MyScore < alpha? alpha: MyScore);
				}
				if(beta <= alpha) {
					return (player == init_player? alpha: beta);
				}
				move = board_size*i+j+1;
			}
		}
	}
	return move == -1? 0 : (player == init_player? alpha: beta);
}

struct game_settings *parse_cmd_args(int argc, char *argv[]) {
	struct game_settings *result = init_game_settings();
	result->board_size = 3;
	result->ff_sign = '#';
	result->language = "en-gb";
	result->p1_name = "Player 1";
	result->p1_sign = 'X';
	result->p2_name = "Player 2";
	result->p2_sign = 'O';
	result->AI_name = "AI";
	for(int i = 1; i < argc; i++){
		if( strncmp(argv[i],"-",1) == 0){
			if(argc - i > 1 && (strcmp(argv[i],"-l") == 0 ||
				strcmp(argv[i],"--language") == 0) && strncmp(argv[i+1],"--",2) != 0) {
				result->language = argv[i+1];
				i++;
			} else if(argc - i > 1 && strcmp(argv[i],"--p1_name") == 0
					&& strncmp(argv[i+1],"--",2) != 0 ) {
				result->p1_name = argv[i+1];
				i++;
			} else if(argc - i > 1 && strcmp(argv[i],"--p2_name") == 0
					&& strncmp(argv[i+1],"--",2) != 0 ) {
				result->p2_name = argv[i+1];
				i++;
			} else if(argc - i > 1 && strcmp(argv[i],"--AI_name") == 0
					&& strncmp(argv[i+1],"--",2) != 0 ) {
				result->AI_name = argv[i+1];
				i++;
			} else if(argc - i > 1 && (strcmp(argv[i],"--b_size") == 0 ||
				strcmp(argv[i],"-s") == 0) && strncmp(argv[i+1],"--",2) != 0 ) {
					result->board_size = strtol(argv[i+1],NULL,10);
					i++;
			} else if(argc - i > 1 && (strcmp(argv[i],"--p1_sign") == 0) &&
					strlen(argv[i+1]) == 1){
					result->p1_sign = argv[i+1][0];
					i++;
			} else if(argc - i > 1 && (strcmp(argv[i],"--p2_sign") == 0) &&
					strlen(argv[i+1]) == 1){
					result->p2_sign = argv[i+1][0];
					i++;
			} else if(argc - i > 1 && (strcmp(argv[i],"--ff_sign") == 0) &&
					strlen(argv[i+1]) == 1){
					result->ff_sign = argv[i+1][0];
					i++;
			} else {
				printf("%s: Unknown command-line parameter: %s\n",argv[0],argv[i]);
			}
		}  else {
				printf("%s: Unknown command-line argument: %s\n",argv[0],argv[i]);
		}
	}
	result->game_strings = (char**)strings(result->language);
	return result;
}
