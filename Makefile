# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eboulajd <eboulajd@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/26 15:17:09 by eboulajd          #+#    #+#              #
#    Updated: 2026/07/27 00:53:20 by eboulajd         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

MAIN = main.c

CC = cc

CODEXH = codexion.h

CFLAGS = -Wall -Wextra -Werror

CODEXC = parser.c \
		 routing.c \
		 simulation.c \
		 init.c \
		 

run:
	$(CC) $(MAIN) $(CODEXC)