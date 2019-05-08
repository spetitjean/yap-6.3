
%% @file yapi.yap
%% @brief support yap shell
%%
%:- start_low_level_trace.
 %% :- module(yapi, [
 %% 		 python_ouput/0,
 %% 		 show_answer/2,
 %% 		 show_answer/3,
 %% 		 yap_query/4,
 %% 		 python_query/2,
 %% 		 python_query/3,
 %% 		 python_import/1,
 %% 		 yapi_query/2
 %% 		 ]).

:- yap_flag(verbose, silent).

 :- use_module(library(python)).

:- use_module( library(lists) ).
:- use_module( library(maplist) ).
:- use_module( library(rbtrees) ).
:- use_module( library(terms) ).


:- python_import(yap4py.yapi).
%:- python_import(gc).

:- meta_predicate( yapi_query(:,+) ).

%:- start_low_level_trace.

	%% @pred yapi_query( + VarList, - Dictionary)
	%%
	%% dictionary, Examples
	%%
	%%
	yapi_query( VarNames, Self ) :-
		show_answer(VarNames, Dict),
		Self.bindings := Dict.


%:- initialization set_preds.

set_preds :-
fail,
	current_predicate(P, Q),
	functor(Q,P,A),
	atom_string(P,S),
	catch(
	      := yap4py.yapi.named( S, A),
	      _,
	      fail),
	fail.
set_preds :-
fail,
	system_predicate(P/A),
	atom_string(P,S),
	catch(
	      := yap4py.yapi.named( S, A),
	      _,
	      fail),
	fail.
set_preds.

argi(N,I,I1) :-
    atomic_concat(`A`,I,N),
	I1 is I+1.

python_query( Caller, String ) :-
	atomic_to_term( String, Goal, VarNames ),
	query_to_answer( Goal, VarNames, Status, Bindings),
	Caller.port := Status,
	write_query_answer( Bindings ),
	nl(user_error),
	Caller.answer := {},
	maplist(in_dict(Caller.answer), Bindings).


in_dict(Dict, var([V0,V|Vs])) :- !,
	Dict[V] := V0,
	in_dict( Dict, var([V0|Vs])).
in_dict(_Dict, var([_],_G)) :- !.
in_dict(Dict, nonvar([V0|Vs],G)) :- !,
term_to_atom(G,A,_),
	Dict[V0] := A,
	in_dict( Dict, nonvar(Vs, G) ).
in_dict(_Dict, nonvar([],_G)) :- !.
in_dict(_, _)
