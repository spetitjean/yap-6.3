///
/// @file YapErrors.h
///
/// @addtogroup YapError
///
/// The file  YapErrors.h contains a list with all the error classes known
/// internally to the YAP system.

BEGIN_ERROR_CLASSES()

/// base case
ECLASS(YAPC_NO_ERROR, "no_error", 0)
/// bad domain, first argument often is the predicate.
ECLASS(DOMAIN_ERROR, "domain_error", 2)
/// bad arithmetic
ECLASS(EVALUATION_ERROR, "evaluation_error", 1)
/// missing object (I/O mostly)
ECLASS(EXISTENCE_ERROR, "existence_error", 2)
/// should be bound
ECLASS(INSTANTIATION_ERROR_CLASS, "instantiation_error", 0)
/// bad access, I/O
ECLASS(PERMISSION_ERROR, "permission_error", 3)
/// something that could not be represented into a type
ECLASS(REPRESENTATION_ERROR, "representation_error", 0)
/// not enough ....
ECLASS(RESOURCE_ERROR, "resource_error", 2)
/// bad text
ECLASS(SYNTAX_ERROR_CLASS, "syntax_error", 1)
/// OS or internal
ECLASS(SYSTEM_ERROR_CLASS, "system_error", 1)
/// bad typing
ECLASS(TYPE_ERROR, "type_error", 2)
/// should be unbound
ECLASS(UNINSTANTIATION_ERROR_CLASS, "uninstantiation_error", 1)
/// not quite an error, but almost
ECLASS(WARNING, "warning", 1)
/// user defined escape hatch
ECLASS(EVENT, "event", 1)

END_ERROR_CLASSES();

BEGIN_ERRORS()

/* ISO_ERRORS */

E0(YAP_NO_ERROR, YAPC_NO_ERROR) /// default state

E(DOMAIN_ERROR_ABSOLUTE_FILE_NAME_OPTION, DOMAIN_ERROR,
  "absolute_file_name_option")
E(DOMAIN_ERROR_ARRAY_OVERFLOW, DOMAIN_ERROR, "array_overflow")
E(DOMAIN_ERROR_ARRAY_TYPE, DOMAIN_ERROR, "array_type")
E(DOMAIN_ERROR_CLOSE_OPTION, DOMAIN_ERROR, "close_option")
E(DOMAIN_ERROR_CREATE_ARRAY_OPTION, DOMAIN_ERROR, "create_array_option")
E(DOMAIN_ERROR_ENCODING, DOMAIN_ERROR, "encoding")
E(DOMAIN_ERROR_EXPAND_FILENAME_OPTION, DOMAIN_ERROR, "expand_filename")
E(DOMAIN_ERROR_FILE_ERRORS, DOMAIN_ERROR, "file_errors")
E(DOMAIN_ERROR_FILE_TYPE, DOMAIN_ERROR, "file_type")
E(DOMAIN_ERROR_FORMAT_CONTROL_SEQUENCE, DOMAIN_ERROR, "format argument")
E(DOMAIN_ERROR_FORMAT_OUTPUT, DOMAIN_ERROR, "format output")
E(DOMAIN_ERROR_GENERIC_ARGUMENT, DOMAIN_ERROR, "generic_argument")
E(DOMAIN_ERROR_IO_MODE, DOMAIN_ERROR, "io_mode")
E(DOMAIN_ERROR_MUTABLE, DOMAIN_ERROR, "mutable")
E(DOMAIN_ERROR_NON_EMPTY_LIST, DOMAIN_ERROR, "non_empty_list")
E(DOMAIN_ERROR_NOT_LESS_THAN_ZERO, DOMAIN_ERROR, "not_less_than_zero")
E(DOMAIN_ERROR_NOT_NL, DOMAIN_ERROR, "not_nl")
E(DOMAIN_ERROR_NOT_ZERO, DOMAIN_ERROR, "not_zero")
E(DOMAIN_ERROR_OPEN_OPTION, DOMAIN_ERROR, "open_option")
E(DOMAIN_ERROR_OPERATOR_PRIORITY, DOMAIN_ERROR, "operator_priority")
E(DOMAIN_ERROR_OPERATOR_SPECIFIER, DOMAIN_ERROR, "operator_specifier")
E(DOMAIN_ERROR_ORDER, DOMAIN_ERROR, "order")
E(DOMAIN_ERROR_OUT_OF_RANGE, DOMAIN_ERROR, "out_of_range")
E(DOMAIN_ERROR_PROLOG_FLAG, DOMAIN_ERROR, "prolog_flag")
E(DOMAIN_ERROR_RADIX, DOMAIN_ERROR, "radix")
E(DOMAIN_ERROR_READ_OPTION, DOMAIN_ERROR, "read_option")
E(DOMAIN_ERROR_SET_STREAM_OPTION, DOMAIN_ERROR, "set_stream_option")
E(DOMAIN_ERROR_SHIFT_COUNT_OVERFLOW, DOMAIN_ERROR, "shift_count_overflow")
E(DOMAIN_ERROR_SOURCE_SINK, DOMAIN_ERROR, "source_sink")
E(DOMAIN_ERROR_SOLUTIONS, DOMAIN_ERROR, "solutions")
E(DOMAIN_ERROR_STREAM, DOMAIN_ERROR, "stream")
E(DOMAIN_ERROR_STREAM_ENCODING, DOMAIN_ERROR, "stream_encoding")
E(DOMAIN_ERROR_STREAM_OPTION, DOMAIN_ERROR, "stream_option")
E(DOMAIN_ERROR_STREAM_OR_ALIAS, DOMAIN_ERROR, "stream_or_alias")
E(DOMAIN_ERROR_STREAM_POSITION, DOMAIN_ERROR, "stream_position")
E(DOMAIN_ERROR_STREAM_PROPERTY_OPTION, DOMAIN_ERROR, "stream_property_option")
E(DOMAIN_ERROR_TIMEOUT_SPEC, DOMAIN_ERROR, "timeout_spec")
E(DOMAIN_ERROR_SYNTAX_ERROR_HANDLER, DOMAIN_ERROR, "syntax_error_handler")
E(DOMAIN_ERROR_WRITE_OPTION, DOMAIN_ERROR, "write_option")

    E(EVALUATION_ERROR_DBMS, EVALUATION_ERROR, "DBMS_error")
    E(EVALUATION_ERROR_FLOAT_OVERFLOW, EVALUATION_ERROR, "float_overflow")
E(EVALUATION_ERROR_FLOAT_UNDERFLOW, EVALUATION_ERROR, "float_underflow")
E(EVALUATION_ERROR_INT_OVERFLOW, EVALUATION_ERROR, "int_overflow")
E(EVALUATION_ERROR_READ_STREAM, EVALUATION_ERROR, "read_from_stream")
E(EVALUATION_ERROR_UNDEFINED, EVALUATION_ERROR, "undefined")
E(EVALUATION_ERROR_UNDERFLOW, EVALUATION_ERROR, "underflow")
    E(EVALUATION_ERROR_ZERO_DIVISOR, EVALUATION_ERROR, "zero_divisor")

E(EXISTENCE_ERROR_ARRAY, EXISTENCE_ERROR, "array")
E(EXISTENCE_ERROR_KEY, EXISTENCE_ERROR, "key, ")
E(EXISTENCE_ERROR_MUTEX, EXISTENCE_ERROR, "mutex, ")
E(EXISTENCE_ERROR_SOURCE_SINK, EXISTENCE_ERROR, "source_sink")
E(EXISTENCE_ERROR_STREAM, EXISTENCE_ERROR, "stream")
E(EXISTENCE_ERROR_VARIABLE, EXISTENCE_ERROR, "variable")

E0(INSTANTIATION_ERROR, INSTANTIATION_ERROR_CLASS)

E2(PERMISSION_ERROR_ACCESS_PRIVATE_PROCEDURE, PERMISSION_ERROR, "access",
   "private_procedure")
E2(PERMISSION_ERROR_NEW_ALIAS_FOR_STREAM, PERMISSION_ERROR, "add_alias",
   "stream")
E2(PERMISSION_ERROR_CREATE_ARRAY, PERMISSION_ERROR, "create", "array")
E2(PERMISSION_ERROR_CREATE_OPERATOR, PERMISSION_ERROR, "create", "operator")
E2(PERMISSION_ERROR_INPUT_BINARY_STREAM, PERMISSION_ERROR, "input",
   "text_stream")
E2(PERMISSION_ERROR_INPUT_PAST_END_OF_STREAM, PERMISSION_ERROR, "input",
   "past_end_of_stream")
E2(PERMISSION_ERROR_INPUT_STREAM, PERMISSION_ERROR, "input", "stream")
E2(PERMISSION_ERROR_INPUT_TEXT_STREAM, PERMISSION_ERROR, "input", "text_stream")
E2(PERMISSION_ERROR_MODIFY_STATIC_PROCEDURE, PERMISSION_ERROR, "modify",
   "static_procedure")
E2(PERMISSION_ERROR_MODULE_REDEFINED, PERMISSION_ERROR, "redefined", "module")
E2(PERMISSION_ERROR_OPEN_SOURCE_SINK, PERMISSION_ERROR, "open", "source_sink")
E2(PERMISSION_ERROR_OUTPUT_BINARY_STREAM, PERMISSION_ERROR, "output",
   "binary_stream")
E2(PERMISSION_ERROR_OUTPUT_STREAM, PERMISSION_ERROR, "output", "stream")
E2(PERMISSION_ERROR_OUTPUT_TEXT_STREAM, PERMISSION_ERROR, "output",
   "text_stream")
E2(PERMISSION_ERROR_READ_ONLY_FLAG, PERMISSION_ERROR, "read_only", "flag")
E2(PERMISSION_ERROR_RESIZE_ARRAY, PERMISSION_ERROR, "resize", "array")
E2(PERMISSION_ERROR_REPOSITION_STREAM, PERMISSION_ERROR, "reposition", "stream")

E(REPRESENTATION_ERROR_CHARACTER, REPRESENTATION_ERROR, "character")
E(REPRESENTATION_ERROR_CHARACTER_CODE, REPRESENTATION_ERROR, "character_code")
E(REPRESENTATION_ERROR_IN_CHARACTER_CODE, REPRESENTATION_ERROR,
  "in_character_code")
E(REPRESENTATION_ERROR_INT, REPRESENTATION_ERROR, "int")
E(REPRESENTATION_ERROR_MAX_ARITY, REPRESENTATION_ERROR, "max_arity")
E(REPRESENTATION_ERROR_VARIABLE, REPRESENTATION_ERROR, "variable")

E(RESOURCE_ERROR_HUGE_INT, RESOURCE_ERROR, "huge_int")
E(RESOURCE_ERROR_MAX_STREAMS, RESOURCE_ERROR, "max_streams")
E(RESOURCE_ERROR_MAX_THREADS, RESOURCE_ERROR, "max_threads")
E(RESOURCE_ERROR_AUXILIARY_STACK, RESOURCE_ERROR, "auxiliary_stack")
E(RESOURCE_ERROR_ATTRIBUTED_VARIABLES, RESOURCE_ERROR, "attributed_variables")
E(RESOURCE_ERROR_HEAP, RESOURCE_ERROR, "database_space")
E(RESOURCE_ERROR_TRAIL, RESOURCE_ERROR, "trail_space")
E(RESOURCE_ERROR_STACK, RESOURCE_ERROR, "stack_space")

E1(SYNTAX_ERROR, SYNTAX_ERROR_CLASS, "syntax_error")
E1(SYNTAX_ERROR_NUMBER, SYNTAX_ERROR_CLASS, "syntax_error")

E(SYSTEM_ERROR_INTERNAL, SYSTEM_ERROR_CLASS, "internal")
E(SYSTEM_ERROR_COMPILER, SYSTEM_ERROR_CLASS, "compiler")
E(SYSTEM_ERROR_FATAL, SYSTEM_ERROR_CLASS, "fatal")
E(SYSTEM_ERROR_GET_FAILED, SYSTEM_ERROR_CLASS, "get_failed")
E(SYSTEM_ERROR_PUT_FAILED, SYSTEM_ERROR_CLASS, "put_failed")
E(SYSTEM_ERROR_JIT_NOT_AVAILABLE, SYSTEM_ERROR_CLASS, "jit_not_available")
E(SYSTEM_ERROR_RUNTIME_ANDROID, SYSTEM_ERROR_CLASS, "android_runtime error")
E(SYSTEM_ERROR_RUNTIME_JAVA, SYSTEM_ERROR_CLASS, "java_runtime error")
E(SYSTEM_ERROR_RUNTIME_JS, SYSTEM_ERROR_CLASS, "javascript_runtime error")
E(SYSTEM_ERROR_RUNTIME_PYTHON, SYSTEM_ERROR_CLASS, "python_runtime error")
E(SYSTEM_ERROR_RUNTIME_R, SYSTEM_ERROR_CLASS, "r_runtime error")
E(SYSTEM_ERROR_OPERATING_SYSTEM, SYSTEM_ERROR_CLASS, "operating_system_error")
E(SYSTEM_ERROR_SAVED_STATE, SYSTEM_ERROR_CLASS, "saved_state_error")

E(ERROR_EVENT, EVENT, "error")
E(ABORT_EVENT, EVENT, "abort")
E(THROW_EVENT, EVENT, "throw")
E(CALL_COUNTER_UNDERFLOW_EVENT, EVENT, "call_counter_underflow")
E(PRED_ENTRY_COUNTER_UNDERFLOW_EVENT, EVENT, "pred_entry_counter_underflow")
E(RETRY_COUNTER_UNDERFLOW_EVENT, EVENT, "retry_counter_underflow")
E(INTERRUPT_EVENT, EVENT, "interrupt")
E(USER_EVENT, EVENT, "user event")

E(TYPE_ERROR_ARRAY, TYPE_ERROR, "array")
E(TYPE_ERROR_ATOM, TYPE_ERROR, "atom")
E(TYPE_ERROR_ATOMIC, TYPE_ERROR, "atomic")
E(TYPE_ERROR_BIGNUM, TYPE_ERROR, "bignum")
E(TYPE_ERROR_BOOLEAN, TYPE_ERROR, "boolean")
E(TYPE_ERROR_BYTE, TYPE_ERROR, "byte")
E(TYPE_ERROR_CALLABLE, TYPE_ERROR, "callable")
E(TYPE_ERROR_CHAR, TYPE_ERROR, "char")
E(TYPE_ERROR_CHARACTER, TYPE_ERROR, "character")
E(TYPE_ERROR_CHARACTER_CODE, TYPE_ERROR, "character_code")
E(TYPE_ERROR_COMPOUND, TYPE_ERROR, "compound")
E(TYPE_ERROR_DBREF, TYPE_ERROR, "dbref")
E(TYPE_ERROR_DBTERM, TYPE_ERROR, "dbterm")
E(TYPE_ERROR_EVALUABLE, TYPE_ERROR, "evaluable")
E(TYPE_ERROR_FLOAT, TYPE_ERROR, "float")
E(TYPE_ERROR_IN_BYTE, TYPE_ERROR, "in_byte")
E(TYPE_ERROR_IN_CHARACTER, TYPE_ERROR, "in_character")
E(TYPE_ERROR_INTEGER, TYPE_ERROR, "integer")
E(TYPE_ERROR_KEY, TYPE_ERROR, "key")
E(TYPE_ERROR_LIST, TYPE_ERROR, "list")
E(TYPE_ERROR_NUMBER, TYPE_ERROR, "number")
E(TYPE_ERROR_PARAMETER, TYPE_ERROR, "parameter")
E(TYPE_ERROR_PREDICATE_INDICATOR, TYPE_ERROR, "predicate_indicator")
E(TYPE_ERROR_PTR, TYPE_ERROR, "pointer")
E(TYPE_ERROR_REFERENCE, TYPE_ERROR, "reference")
E(TYPE_ERROR_STRING, TYPE_ERROR, "string")
E(TYPE_ERROR_TEXT, TYPE_ERROR, "text")
E(TYPE_ERROR_UBYTE, TYPE_ERROR, "ubyte")
E(TYPE_ERROR_UCHAR, TYPE_ERROR, "unsigned char")

E(WARNING_DISCONTIGUOUS, WARNING, "discontiguous")
E(WARNING_SINGLETONS, WARNING, "singletons")
E(WARNING_SYNTAX_ERROR, WARNING, "syntax_error")

E1(UNINSTANTIATION_ERROR, UNINSTANTIATION_ERROR_CLASS, "uninstantiation_error")

END_ERRORS();
