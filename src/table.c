
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "table.h"
#include "logging.h"
#include "player.h"

void init_new_game(table *t);

table *table_new(char *name)
{
	table *t = malloc(sizeof(table));
	
	t->name = name;
	t->state = WAITING_FOR_PLAYERS;
	t->players = malloc(sizeof(player*) * 3);
	t->num_players = 0;
	t->current_player = 0;

	return t;
}

void table_add_player(table *t, player *p)
{

	if (t->num_players == 3)
	{
		logging_critical("Can't add player %s to table %s because limit has been reached.", p->name, t->name);
		return;
	}

	*(t->players + t->num_players) = p;
	t->num_players++;

	send_str(p->socket, "Welcome to the table. Please wait for others to join");
	logging_debug("Now %i players on table %s", t->num_players, t->name);
	table_broadcast(t, "Player %s has joined!", p->name);

	if (t->num_players == 3)
		init_new_game(t);

}

void table_broadcast(table *t, char *message, ...)
{
	va_list ap;
	char arg_merged[250];

	player *p;
	int i;
		
	va_start(ap, message);
	vsnprintf(arg_merged, 250, message, ap);
	va_end(ap);

	for (i = 0; i < t->num_players; i++)
	{
		p = t->players[i];
		send_str(p->socket, arg_merged);
	}
}

void init_new_game(table *t)
{
	t->state = IN_PROGRESS;
	t->current_player = 0;

	logging_debug("GAME STARTED!!!!! DEALING CARDS OUT");
	table_broadcast(t, "The game has started!!!");
}

void table_state_changed(table *t, player *p)
{
	char *s;
	logging_debug("game logic for table %s goes here!", t->name);
	s = recv_str(p->socket);
	logging_debug("%s sent %s", p->name, recv_str(p->socket));
}
