	:- module(test_answer,
	  [ test_answer/0,
	    test_answer/2
	  ]).

% :- if(absolute_file_name(foreign(unix), _,
% 	       		 [ file_type(executable),
% 	       		   file_errors(fail),
% 	       		   access(read)
% 	       		 ])).

:- use_module(library(plunit)).
:- use_module(library(unix)).
:- use_module(library(error)).
:- use_module(library(debug)).
:- use_module(library(lists)).
:- use_module(library(maplist)).
%:- use_module(library(dif)).

/** <module> Test unit for toplevel replies

This module tests the way in which  the toplevel replies to queries. The
implementation is based on pipes, which   are  provided by library(unix)
from the clib package. This means that   this  test can only be executed
after library(unix) has been compiled.
*/

test_answer :-
	run_tests([ answer
		  ]).

%%	toplevel_answer(+GoalAtom, -Answer:string) is det.
%
%	Run GoalAtom in a seperate thread and   catch the output that is
%	produces by Prolog.

toplevel_answer(Goal, Answer) :-
	strip_module( Goal, M, G),
	atom_to_term(G, GGoal, Bindings),
	call(M:GGoal),
	maplist(mkatom, Bindings, Answer).

mkatom(X=T, NT) :-
	atom_to_term(NT, X=T, _ ).

%%	test_answer(+Query, -OkReplies) is semidet.
%
%	True if Query produces one of the outputs in OkReplies.

test_answer(QueryAtom, Replies) :-
	toplevel_answer(QueryAtom, Replies).
/*	debug(test_answer, 'Got: ~q', [Output]),
	member(ReplyAtom, Replies),
	term_string(Reply, ReplyAtom,
		    [ variable_names(ReplyBindings0),
		      comments(ReplyComments)
		    ]),
	maplist(anon_binding, OutBindings, ReplyBindings0, ReplyBindings),
	(   debug(test_answer, 'Comments: ~p vs ~p',
		  [OutComments, ReplyComments]),
	    maplist(compare_comment, OutComments, ReplyComments)
	->  true
	;   debug(test_answer, '~p', [ OutComments \= ReplyComments ]),
	    fail
	),
	(   Written+OutBindings =@= Reply+ReplyBindings
	->  true
	;   debug(test_answer, '~q',
		  [ Written+OutBindings \=@= Reply+ReplyBindings ]),
	    fail
	).
*/


anon_binding(Name=_, GName=Var, Name=Var) :-
	sub_atom(GName, 0, _, _, '_G'), !.
anon_binding(_, Binding, Binding).

compare_comment(_-C, _-C).

hidden :-
	dif(_X, a).

:- begin_tests(answer, [sto(rational_trees)]).

test(simple, true) :-
	test_answer('A=1', ['A=1']).
test(simple, true) :-
	test_answer('A=1, B=2', ['A=1, B=2']).
test(separated, true) :-
	test_answer('X = 2, Y = 1, Z = 2', ['X = Z, Z = 2, Y = 1']).
test(same, true) :-
	test_answer('A=1, B=1', ['A=B, B=1']).
test(same, true) :-
	test_answer('A=a(B), B=A', ['A=B, B=a(B)']).
test(cycle, true) :-
	test_answer('A=a(A)', ['A=a(A)']).
test(cycle, true) :-
	test_answer('A=a(A), B=a(a(B))', ['A=B, B=a(a(B))']).
test(double_cycle, true) :-
	test_answer('X = s(X,Y), Y = s(X,X)',
		    [ 'X = Y, Y = s(_S1, _S1), % where
		          _S1 = s(_S1, s(_S1, _S1))'
		    ]).
test(freeze, true) :-
	test_answer('freeze(X, writeln(X))', ['freeze(X, writeln(X))']).
test(hidden, true) :-
	test_answer('test_answer:hidden',
		    [ '% with pending residual goals
		      dif(_G1,a)'
		    ]).
test(hidden, true) :-
	test_answer('test_answer:hidden, A = a',
		    [ 'A = a,
		      % with pending residual goals
		      dif(_G1,a)'
		    ]).
test(hidden, true) :-
	test_answer('test_answer:hidden, A = a, dif(B, b)',
		    [ 'A = a,
		      dif(B, b),
		      % with pending residual goals
		      dif(_G1,a)'
		    ]).
:- end_tests(answer).

/*

:- else.				% No foreign(unix) found

test_answer :-
	format(user_error, 'Skipped toplevel answer tests; requires clib~n', []).

test_answer(_QueryAtom, _Replies).	% satisfy exports

:- endif.
*/
