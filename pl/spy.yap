/**
 * @file spy.yap
 * @brief debugger operation.
 */
  :- system_module( '$_debug', [debug/0,
        debugging/0,
        leash/1,
        nodebug/0,
        (nospy)/1,
        nospyall/0,
        notrace/0,
        (spy)/1,
        trace/0], [
        '$init_debugger'/0]).

:- use_system_module( '$_boot', ['$find_goal_definition'/4,
        '$system_catch'/4]).

:- use_system_module( '$_errors', ['$Error'/1,
        '$do_error'/2]).

:- use_system_module( '$_init', ['$system_module'/1]).

:- use_system_module( '$_modules', ['$meta_expansion'/6]).

:- use_system_module( '$_preds', ['$clause'/4]).

/*-----------------------------------------------------------------------------

			Debugging / creating spy points

-----------------------------------------------------------------------------*/

/**
 * @defgroup DebSet Debugger Control
 * @ingroup Deb_Interaction

@{
The
 following predicates are available to control the debugging of
programs:

+ debug

    Switches the debugger on.

+ debugging


    Outputs status information about the debugger which includes the leash
mode and the existing spy-points, when the debugger is on.

+ nodebug

    Switches the debugger off.


*/


:- op(900,fx,[spy,nospy]).

% First part : setting and reseting spy points

 % $suspy does most of the work
 '$suspy'(V,S,M) :- var(V) , !,
	 '$do_error'(instantiation_error,M:spy(V,S)).
 '$suspy'((M:S),P,_) :- !,
     '$suspy'(S,P,M).
 '$suspy'([],_,_) :- !.
 '$suspy'([F|L],S,M) :- !, ( '$suspy'(F,S,M) ; '$suspy'(L,S,M) ).
 '$suspy'(F/N,S,M) :- !,
	 functor(T,F,N),
	 '$do_suspy'(S, F, N, T, M).
 '$suspy'(A,S,M) :- atom(A), !,
	 '$suspy_predicates_by_name'(A,S,M).
 '$suspy'(P,spy,M) :- !,
	  '$do_error'(domain_error(predicate_spec,P),spy(M:P)).
 '$suspy'(P,nospy,M) :-
	  '$do_error'(domain_error(predicate_spec,P),nospy(M:P)).

 '$suspy_predicates_by_name'(A,S,M) :-
	 % just check one such predicate exists
	 current_predicate(A,M:_),
	 !,
	 '$do_suspy_predicates_by_name'(A,S,M).
'$suspy_predicates_by_name'(A,spy,M) :- !,
	 print_message(warning,no_match(spy(M:A))).
'$suspy_predicates_by_name'(A,nospy,M) :-
	 print_message(warning,no_match(nospy(M:A))).

'$do_suspy_predicates_by_name'(A,S,M) :-
	 current_predicate(A,M:T),
	 functor(T,A,N),
	 '$do_suspy'(S, A, N, T, M),
	 fail.
'$do_suspy_predicates_by_name'(_A, _S, _M).


 %
 % protect against evil arguments.
 %
 '$do_suspy'(S, F, N, T, M) :-
	 recorded('$import','$import'(EM,M,T0,_,F,N),_), !,
	 functor(T0, F0, N0),
	 '$do_suspy'(S, F0, N0, T, EM).
 '$do_suspy'(S, F, N, T, M) :-
	  '$undefined'(T,M), !,
	  ( S = spy ->
	      print_message(warning,no_match(spy(M:F/N)))
	  ;
	      print_message(warning,no_match(nospy(M:F/N)))
	  ).
 '$do_suspy'(S, F, N, T, M) :-
	  '$is_system_predicate'(T,M),
	  '$predicate_flags'(T,M,F,F),
	  F /\ 0x118dd080 =\= 0,
	  ( S = spy ->
	      '$do_error'(permission_error(access,private_procedure,T),spy(M:F/N))
	  ;
	      '$do_error'(permission_error(access,private_procedure,T),nospy(M:F/N))
	  ).
 '$do_suspy'(S, F, N, T, M) :-
	  '$undefined'(T,M), !,
	  ( S = spy ->
	      print_message(warning,no_match(spy(M:F/N)))
	  ;
	      print_message(warning,no_match(nospy(M:F/N)))
	  ).
 '$do_suspy'(S,F,N,T,M) :-
	 '$suspy2'(S,F,N,T,M).

 '$suspy2'(spy,F,N,T,M) :-
	 recorded('$spy','$spy'(T,M),_), !,
	 print_message(informational,breakp(bp(debugger,plain,M:T,M:F/N,N),add,already)).
 '$suspy2'(spy,F,N,T,M) :- !,
	 recorda('$spy','$spy'(T,M),_),
	 '$set_spy'(T,M),
	 print_message(informational,breakp(bp(debugger,plain,M:T,M:F/N,N),add,ok)).
 '$suspy2'(nospy,F,N,T,M) :-
	 recorded('$spy','$spy'(T,M),R), !,
	 erase(R),
	 '$rm_spy'(T,M),
	 print_message(informational,breakp(bp(debugger,plain,M:T,M:F/N,N),remove,last)).
 '$suspy2'(nospy,F,N,_,M) :-
	 print_message(informational,breakp(no,breakpoint_for,M:F/N)).

 '$pred_being_spied'(G, M) :-
	 recorded('$spy','$spy'(G,M),_), !.

/**
@pred spy( + _P_ ).

Sets spy-points on all the predicates represented by
 _P_.  _P_ can either be a single specification or a list of
specifications. Each one must be of the form  _Name/Arity_
or  _Name_. In the last case all predicates with the name
 _Name_ will be spied. As in C-Prolog, system predicates and
predicates written in C, cannot be spied.


*/
 spy Spec :-
	 '$init_debugger',
	 prolog:debug_action_hook(spy(Spec)), !.
 spy L :-
	 '$current_module'(M),
	 '$suspy'(L, spy, M), fail.
 spy _ :- debug.

/** @pred nospy( + _P_ )


Removes spy-points from all predicates specified by  _P_.
The possible forms for  _P_ are the same as in `spy P`.


*/
 nospy Spec :-
	 '$init_debugger',
	 prolog:debug_action_hook(nospy(Spec)), !.
 nospy L :-
	 '$current_module'(M),
	 '$suspy'(L, nospy, M), fail.
nospy _.

/** @pred nospyall

Removes all existing spy-points.
*/
nospyall :-
	 '$init_debugger',
	 prolog:debug_action_hook(nospyall), !.
nospyall :-
	 recorded('$spy','$spy'(T,M),_), functor(T,F,N), '$suspy'(F/N,nospy,M), fail.
nospyall.

 % debug mode -> debug flag = 1

debug :-
	 '$init_debugger',
	 ( '__NB_getval__'('$spy_gn',_, fail) -> true ; '__NB_setval__'('$spy_gn',1) ),
	 set_prolog_flag(debug,true),
	 '$start_user_code',
	 print_message(informational,debug(debug)).

'$start_user_code' :-
    yap_flag(debug, Can),
    '$set_debugger_state'(debug, Can),
    '__NB_getval__'('$trace',Trace, fail),
    ( Trace == on -> Creep = creep; Creep = zip ),
    '$set_debugger_state'( Creep, 0, stop, Trace ).
nodebug :-
	 set_prolog_flag(debug, false),
	 '$init_debugger',
	 '__NB_setval__'('$trace',off),
	 print_message(informational,debug(off)).

%
% remove any debugging info after an abort.
%


/** @pred trace


Switches on the debugger and enters tracing mode.


*/
trace :-
    print_message(informational,debug(trace)),
    set_prolog_flag(debug,true),
    '__NB_setval__'('$trace',on),
    '$init_debugger'.

/** @pred notrace


Ends tracing and exits the debugger. This is the same as
nodebug/0.
 */
notrace :-
	'$init_debugger',
	nodebug.

/*-----------------------------------------------------------------------------

				leash

-----------------------------------------------------------------------------*/


/** @pred leash(+ _M_)


Sets leashing mode to  _M_.
The mode can be specified as:

+ `full`
prompt on Call, Exit, Redo and Fail

+ `tight`
prompt on Call, Redo and Fail

+ `half`
prompt on Call and Redo

+ `loose`
prompt on Call

+ `off`
never prompt

+ `none`
never prompt, same as `off`

The initial leashing mode is `full`.

The user may also specify directly the debugger ports
where he wants to be prompted. If the argument for leash
is a number  _N_, each of lower four bits of the number is used to
control prompting at one the ports of the box model. The debugger will
prompt according to the following conditions:

+ if `N/\ 1 =\= 0`  prompt on fail
+ if `N/\ 2 =\= 0` prompt on redo
+ if `N/\ 4 =\= 0` prompt on exit
+ if `N/\ 8 =\= 0` prompt on call

Therefore, `leash(15)` is equivalent to `leash(full)` and
`leash(0)` is equivalent to `leash(off)`.

Another way of using `leash` is to give it a list with the names of
the ports where the debugger should stop. For example,
`leash([call,exit,redo,fail])` is the same as `leash(full)` or
`leash(15)` and `leash([fail])` might be used instead of
`leash(1)`.

 @}

*/
leash(X) :- var(X),
	'$do_error'(instantiation_error,leash(X)).
leash(X) :-
	'$init_debugger',
	'$leashcode'(X,Code),
	set_value('$leash',Code),
	'$show_leash'(informational,Code), !.
leash(X) :-
	'$do_error'(type_error(leash_mode,X),leash(X)).

'$show_leash'(Msg,0) :-
	print_message(Msg,leash([])).
'$show_leash'(Msg,Code) :-
	'$check_leash_bit'(Code,0x8,L3,call,LF),
	'$check_leash_bit'(Code,0x4,L2,exit,L3),
	'$check_leash_bit'(Code,0x2,L1,redo,L2),
	'$check_leash_bit'(Code,0x1,[],fail,L1),
	print_message(Msg,leash(LF)).

'$check_leash_bit'(Code,Bit,L0,_,L0) :- Bit /\ Code =:= 0, !.
'$check_leash_bit'(_,_,L0,Name,[Name|L0]).

'$leashcode'(full,0xf) :- !.
'$leashcode'(on,0xf) :- !.
'$leashcode'(half,0xb) :- !.
'$leashcode'(loose,0x8) :- !.
'$leashcode'(off,0x0) :- !.
'$leashcode'(none,0x0) :- !.
%'$leashcode'([L|M],Code) :- !, '$leashcode_list'([L|M],Code).
'$leashcode'([L|M],Code) :- !,
	'$list2Code'([L|M],Code).
'$leashcode'(N,N) :- integer(N), N >= 0, N =< 0xf.

'$list2Code'(V,_) :- var(V), !,
	'$do_error'(instantiation_error,leash(V)).
'$list2Code'([],0) :- !.
'$list2Code'([V|L],_) :- var(V), !,
	'$do_error'(instantiation_error,leash([V|L])).
'$list2Code'([call|L],N) :- '$list2Code'(L,N1), N is 0x8 + N1.
'$list2Code'([exit|L],N) :- '$list2Code'(L,N1), N is 0x4 + N1.
'$list2Code'([redo|L],N) :- '$list2Code'(L,N1), N is 0x2 + N1.
'$list2Code'([fail|L],N) :- '$list2Code'(L,N1), N is 0x1 + N1.

/*-----------------------------------------------------------------------------

				debugging

-----------------------------------------------------------------------------*/

debugging :-
	'$init_debugger',
	prolog:debug_action_hook(nospyall), !.
debugging :-
	( current_prolog_flag(debug, true) ->
	    print_message(help,debug(debug))
	    ;
	    print_message(help,debug(off))
	),
	findall(M:(N/A),(recorded('$spy','$spy'(T,M),_),functor(T,N,A)),L),
	print_message(help,breakpoints(L)),
	get_value('$leash',Leash),
	'$show_leash'(help,Leash).

notrace(G) :-
	 strip_module(G, M, G1),
	 ( '$$save_by'(CP),
	   '$debug_stop'( State ),
	   '$call'(G1, CP, G, M),
	   '$$save_by'(CP2),
	   (CP == CP2 -> ! ; '$debug_state'( NState ), ( true ; '$debug_restart'(NState), fail ) ),
	   '$debug_restart'( State )
     ;
	'$debug_restart'( State ),
	fail
    ).

'$creep_at_port'(retry) :-
    '$get_debugger_state'(debug, true),
    '__NB_getval__'('$trace',Trace,fail),
    Trace = on,
    !,
    '$enable_debugging'.
'$creep_at_port'(fail) :-
    '$get_debugger_state'(debug, true),
   '__NB_getval__'('$trace',Trace,fail),
    Trace = on,
    !,
    '$enable_debugging'.

'$init_debugger' :- 
    '$debugger_io',
    '$init_debugger_trace',
	'__NB_setval__'('$if_skip_mode',no_skip),
	'__NB_setval__'('$spy_glist',[]),
	'__NB_setval__'('$spy_gn',1).
 
'$init_debugger_trace' :-
	'__NB_getval__'('$trace',on,fail),
	!,
	'$set_debugger_state'( creep,  0, stop, on ).
'$init_debugger_trace' :-
	'__NB_setval__'('$trace',off),
	'$set_debugger_state'( zip, 0, stop, off ).

%% @pred $enter_debugging(G,Mod,CP,G0,NG)
%%
%% Internal predicate called by top-level;
%% enable creeping on a goal by just switching execution to debugger.
%%
'$enter_debugging'(G,Mod,_CP,_G0,_NG) :-
    '$creepcalls'(G,Mod),
    !.
'$enter_debugging'(G,_Mod,_CP,_G0,G).

'$enter_debugging'(G,Mod,GN) :-
    current_prolog_flag( debug, Deb ),
    '__NB_set_value__'( debug, Deb ),
    ( Deb = false
    ->
    true
    ;
    '$creep_is_on_at_entry'(G,Mod,GN)
    ->
    '$creep'
    ;
    true
    ).

'$exit_debugger'(Mod:G, GN) :-
    current_prolog_flag( debug, Deb ),
    '__NB_set_value__'( debug, Deb ),
    ( Deb = false
    ->
    true
    ;
    '$creep_is_on_at_entry'(G,Mod,GN)
    ->
    '$creep'
    ;
    true
    ).

%% we're coming back from external code to a debugger call.
%%
'$reenter_debugger'(fail) :-
    !,
    '$re_enter_creep_mode'.
'$reenter_debugger'(exit) :-
    !,
    '$re_enter_creep_mode'.
'$reenter_debugger'(answer) :-
     !,
   '$re_enter_creep_mode'.
'$reenter_debugger'(_) :-
    '$set_debugger_state'(debug, false).

% what to do when you exit the debugger.
'$continue_debugging'(exit) :-
    !,
    '$re_enter_creep_mode'.
'$continue_debugging'(answer) :-
    !,
    '$re_enter_creep_mode'.			   
'$continue_debugging'(fail) :-
    !,
    '$re_enter_creep_mode',
    fail.
'$continue_debugging'(redo) :-
    !,
    '$re_enter_creep_mode',
    fail.
'$continue_debugging'(_).

'$enable_debugging' :-
    '$re_enter_creep_mode'.
 
 
%% @pred $re_enter_creep_mode1
%%
%% Internal predicate called when exiting through a port;
%% enable creeping on the next goal.
%%
'$re_enter_creep_mode' :-
    current_prolog_flag( debug, Deb ),
    '__NB_setval__'( debug, Deb ).


'$creep_is_off'(Module:G, GoalNo) :-
     (
	 current_prolog_flag( debug, false )
     -> true
    ;
	 '$is_opaque_predicate'(G,Module)
     -> true
    ;
	 '$is_private'(G,Module)
     -> true
    ;
 	'$get_debugger_state'( creep, zip )  ->
    true
    ;
    '$pred_being_spied'(G,Module)
    ->
	'$get_debugger_state'( spy,  ignore )
    ;
    var(GoalNo)
    ->
    false
    ;
	'$get_debugger_state'( goal_number, GN ) ,
	    GN > GoalNo
     ).
'$creep_is_on_at_entry'(G,M,_GoalNo) :-
    \+ '$is_system_predicate'(G,M),
    '$get_debugger_state'(Step, _GN, Spy,_),
    (
	Step \= zip
    ;
    Spy == stop,
    '$pred_being_spied'(G,M)
    ).

   
'$trace_on' :-
    '$get_debugger_state'(_Creep, GN, Spy,Trace),
    nb_setval('$trace',on),
'$set_debugger_state'( creep, GN, Spy, Trace).

'$trace_off' :-
'$get_debugger_state'( _Creep, GN, Spy, Trace),
    nb_setval('$trace',off),
'$set_debugger_state'( creep, GN, Spy, Trace).

/*

@}

*/
