import readline
from yap4py.yap import *
from os.path import join, dirname
from collections import namedtuple
import sys

yap_lib_path = dirname(__file__)

bindvars = namedtuple('bindvars', 'list')
compile = namedtuple('compile', 'file')
jupyter_query = namedtuple('jupyter_query', 'vars dict')
library = namedtuple('library', 'listfiles')
prolog_library = namedtuple('prolog_library', 'listfiles')
python_query = namedtuple('python_query', 'vars dict')
set_prolog_flag = namedtuple('set_prolog_flag', 'flag new_value')
show_answer = namedtuple('show_answer', 'vars dict')
v0 = namedtuple('v', 'slot')
yap_query = namedtuple('yap_query', 'query owner')
yapi_query = namedtuple('yapi_query', 'vars dict')


class Engine( YAPEngine ):

    def __init__(self, args=None,self_contained=False,**kwargs):
        # type: (object) -> object
        if not args:
            args = EngineArgs(**kwargs)
            args.setEmbedded(True)
        if self_contained:
            yap_lib_path = dirname(__file__)
            args.setYapShareDir(join(yap_lib_path, "prolog"))
            args.setYapPLDIR(yap_lib_path)
            args.setSavedState(join(yap_lib_path, "startup.yss"))
        YAPEngine.__init__(self, args)
        self.run(compile(library('yapi')),m="user",release=True)

    def run(self, g, m=None, release=False):
        if m:
            self.mgoal(g, m, release)
        else:
            self.goal(g, release)

    def prolog_library(self, file):
        g = prolog_library(file)
        self.run(g)
            
class JupyterEngine( Engine ):

    def __init__(self, args=None,self_contained=False,**kwargs):
        # type: (object) -> object
        if not args:
            args = EngineArgs(**kwargs)
        args.jupyter = True
        Engine.__init__(self, args)
        self.errors = None
        try:
            self.run(compile(library('jupyter')),"user")
            self.run(compile(library('complete')),"user")
            self.run(compile(library('verify')),"user")
        except:
            pass

class EngineArgs( YAPEngineArgs ):
    """ Interface to Engine Options class"""
    def __init__(self, args=None,**kwargs):
        super().__init__()


class Predicate( YAPPredicate ):
    """ Interface to Generic Predicate"""

    def __init__(self, t, module=None):
        super().__init__(t)

class Query (YAPQuery):
    """Goal is a predicate instantiated under a specific environment """
    def __init__(self, engine, g):
        super().__init__(g)
        self.engine = engine
        self.port = "call"
        self.bindings = None
        self.answer = {}

    def __iter__(self):
        return self

    def __next__(self):
        if self.port == "fail":
            raise IndexError()
        return self.next()
 
def name( name, arity):
    try:
        if  arity > 0 and name.isidentifier(): # and not keyword.iskeyword(name):
            s = []
            for i in range(arity):
                s += ["A" + str(i)]
            return namedtuple(name, s)
    except:
        return None

class PrologPredicate( YAPPrologPredicate ):
    """ Interface to Prolog  Predicate"""

class v(YAPVarTerm,v0):
    def __init__(self):
        YAPVarTerm.__init__()

    def binding(self):
        return self.term()


class YAPShell:

    def numbervars( self ):
        Dict = {}
        self.engine.goal(show_answer( self, Dict), True)
        return Dict
        # rc = self.q.namedVarsVector()
        # self.q.r = self.q.goal().numbervars()
        # o = []
        # for i  in rc:
        #     if len(i) == 2:
        #         do = str(i[0]) + " = " + str( i[1] ) + "\n"
        #         o += do
        #     else:
        #         do = str(i[0]) + " = " + str( i[1] ) + "\n"
        #         o += do
        # return o


    def query_prolog(self, query):
        g = None
        #import pdb; pdb.set_trace()
        #
        # construct a query from a one-line string
        # q is opaque to Python
        #
        #q = engine.query(python_query(self, s))
        #
        #        # vs is the list of variables
        # you can print it out, the left-side is the variable name,
        # the right side wraps a handle to a variable
        import pdb; pdb.set_trace()
        #     #pdb.set_trace()
        # atom match either symbols, or if no symbol exists, sttrings, In this case
        # variable names should match strings
        #for eq in vs:
        #    if not isinstance(eq[0],str):
        #        print( "Error: Variable Name matches a Python Symbol")
        #        return
        try:
            engine = self.engine
            bindings   = []
            loop = False
            g = python_query(self, query)
            self.q = Query( engine, g )
            while self.q.next():
                bindings += [self.q.answer]
                if self.q.port == "exit":
                    break
                if loop:
                    continue
                s = input("more(;), all(*), no(\\n), python(#)?  ").lstrip()
                if s.startswith(';') or s.startswith('y'):
                    continue
                elif s.startswith('#'):
                    try:
                        exec(s.lstrip('#'))
                    except:
                        raise
                elif s.startswith('*') or s.startswith('a'):
                    loop = True
                    continue
                else:
                    break
            if self.q:
                self.q.close()
                self.q = None
            if bindings:
                return True,bindings
            print("No (more) answers")
            return False, None
        except Exception as e:
            if not self.q:
                return False, None
            self.q.close()
            self.q = None
            print("Exception",e)
            e.errorNo = 0
            raise

    def live(self, engine, **kwargs):
        loop = True
        self.q = None
        while loop:
            try:
                s = input("?- ")
                if not s:
                    continue
                else:
                    self.query_prolog(s)
            except SyntaxError as err:
                print("Syntax Error error: {0}".format(err))
                continue
            except EOFError:
                return
            except RuntimeError as err:
                print("YAP Execution Error: {0}".format(err))
            except ValueError:
                print("Could not convert data to an integer.")
            except:
                print("Unexpected error:", sys.exc_info()[0])
                raise
        engine.close()
    #
    # initialize engine
    # engine = yap.YAPEngine();
    # engine = yap.YAPEngine(yap.YAPParams());
    #
    def __init__(self, engine, **kwargs):
        self.engine = engine

        self.live(engine)
        self.q = None


def main():
    engine = Engine()
    YAPShell(engine)

if __name__ == "__main__":
    main()
