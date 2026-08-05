# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/28                                  #+#    #+#          #
#    Updated: 2026/07/28                                  ###   ########.fr    #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC			= cc
CFLAGS = -Wall -Wextra -Werror -pthread -g3
RM			= rm -f

SRCS		=	main.c \
				init.c \
				init_utils.c \
				routine.c \
				simulation.c \
				monitor.c \
				monitor_utils.c \
				parser.c \
				utils.c \
				heap.c \
				heap_utils.c \
				scheduler.c \
				scheduler_utils.c

OBJS		= $(SRCS:.c=.o)

INCLUDES	= -I.

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re