// A Bison parser, made by GNU Bison 3.5.1.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.


// Take the name prefix into account.
#define yylex   parser4_lex



#include "dhcp4_parser.h"


// Unqualified %code blocks.
#line 34 "dhcp4_parser.yy"

#include <dhcp4/parser_context.h>

#line 51 "dhcp4_parser.cc"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if PARSER4_DEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !PARSER4_DEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !PARSER4_DEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 14 "dhcp4_parser.yy"
namespace isc { namespace dhcp {
#line 143 "dhcp4_parser.cc"


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Dhcp4Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  Dhcp4Parser::Dhcp4Parser (isc::dhcp::Parser4Context& ctx_yyarg)
#if PARSER4_DEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      ctx (ctx_yyarg)
  {}

  Dhcp4Parser::~Dhcp4Parser ()
  {}

  Dhcp4Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  Dhcp4Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Dhcp4Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Dhcp4Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Dhcp4Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Dhcp4Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Dhcp4Parser::symbol_number_type
  Dhcp4Parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  Dhcp4Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Dhcp4Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 225: // value
      case 229: // map_value
      case 270: // ddns_replace_client_name_value
      case 292: // socket_type
      case 295: // outbound_interface_value
      case 317: // db_type
      case 411: // hr_mode
      case 569: // ncr_protocol_value
        value.YY_MOVE_OR_COPY< ElementPtr > (YY_MOVE (that.value));
        break;

      case 209: // "boolean"
        value.YY_MOVE_OR_COPY< bool > (YY_MOVE (that.value));
        break;

      case 208: // "floating point"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case 207: // "integer"
        value.YY_MOVE_OR_COPY< int64_t > (YY_MOVE (that.value));
        break;

      case 206: // "constant string"
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Dhcp4Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 225: // value
      case 229: // map_value
      case 270: // ddns_replace_client_name_value
      case 292: // socket_type
      case 295: // outbound_interface_value
      case 317: // db_type
      case 411: // hr_mode
      case 569: // ncr_protocol_value
        value.move< ElementPtr > (YY_MOVE (that.value));
        break;

      case 209: // "boolean"
        value.move< bool > (YY_MOVE (that.value));
        break;

      case 208: // "floating point"
        value.move< double > (YY_MOVE (that.value));
        break;

      case 207: // "integer"
        value.move< int64_t > (YY_MOVE (that.value));
        break;

      case 206: // "constant string"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  Dhcp4Parser::stack_symbol_type&
  Dhcp4Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 225: // value
      case 229: // map_value
      case 270: // ddns_replace_client_name_value
      case 292: // socket_type
      case 295: // outbound_interface_value
      case 317: // db_type
      case 411: // hr_mode
      case 569: // ncr_protocol_value
        value.copy< ElementPtr > (that.value);
        break;

      case 209: // "boolean"
        value.copy< bool > (that.value);
        break;

      case 208: // "floating point"
        value.copy< double > (that.value);
        break;

      case 207: // "integer"
        value.copy< int64_t > (that.value);
        break;

      case 206: // "constant string"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  Dhcp4Parser::stack_symbol_type&
  Dhcp4Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 225: // value
      case 229: // map_value
      case 270: // ddns_replace_client_name_value
      case 292: // socket_type
      case 295: // outbound_interface_value
      case 317: // db_type
      case 411: // hr_mode
      case 569: // ncr_protocol_value
        value.move< ElementPtr > (that.value);
        break;

      case 209: // "boolean"
        value.move< bool > (that.value);
        break;

      case 208: // "floating point"
        value.move< double > (that.value);
        break;

      case 207: // "integer"
        value.move< int64_t > (that.value);
        break;

      case 206: // "constant string"
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  Dhcp4Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if PARSER4_DEBUG
  template <typename Base>
  void
  Dhcp4Parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
    {
      case 206: // "constant string"
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < std::string > (); }
#line 447 "dhcp4_parser.cc"
        break;

      case 207: // "integer"
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < int64_t > (); }
#line 453 "dhcp4_parser.cc"
        break;

      case 208: // "floating point"
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < double > (); }
#line 459 "dhcp4_parser.cc"
        break;

      case 209: // "boolean"
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < bool > (); }
#line 465 "dhcp4_parser.cc"
        break;

      case 225: // value
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 471 "dhcp4_parser.cc"
        break;

      case 229: // map_value
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 477 "dhcp4_parser.cc"
        break;

      case 270: // ddns_replace_client_name_value
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 483 "dhcp4_parser.cc"
        break;

      case 292: // socket_type
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 489 "dhcp4_parser.cc"
        break;

      case 295: // outbound_interface_value
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 495 "dhcp4_parser.cc"
        break;

      case 317: // db_type
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 501 "dhcp4_parser.cc"
        break;

      case 411: // hr_mode
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 507 "dhcp4_parser.cc"
        break;

      case 569: // ncr_protocol_value
#line 290 "dhcp4_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 513 "dhcp4_parser.cc"
        break;

      default:
        break;
    }
    yyo << ')';
  }
#endif

  void
  Dhcp4Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Dhcp4Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Dhcp4Parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if PARSER4_DEBUG
  std::ostream&
  Dhcp4Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Dhcp4Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Dhcp4Parser::debug_level_type
  Dhcp4Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Dhcp4Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // PARSER4_DEBUG

  Dhcp4Parser::state_type
  Dhcp4Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  Dhcp4Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Dhcp4Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Dhcp4Parser::operator() ()
  {
    return parse ();
  }

  int
  Dhcp4Parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (ctx));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case 225: // value
      case 229: // map_value
      case 270: // ddns_replace_client_name_value
      case 292: // socket_type
      case 295: // outbound_interface_value
      case 317: // db_type
      case 411: // hr_mode
      case 569: // ncr_protocol_value
        yylhs.value.emplace< ElementPtr > ();
        break;

      case 209: // "boolean"
        yylhs.value.emplace< bool > ();
        break;

      case 208: // "floating point"
        yylhs.value.emplace< double > ();
        break;

      case 207: // "integer"
        yylhs.value.emplace< int64_t > ();
        break;

      case 206: // "constant string"
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
#line 299 "dhcp4_parser.yy"
                     { ctx.ctx_ = ctx.NO_KEYWORD; }
#line 781 "dhcp4_parser.cc"
    break;

  case 4:
#line 300 "dhcp4_parser.yy"
                      { ctx.ctx_ = ctx.CONFIG; }
#line 787 "dhcp4_parser.cc"
    break;

  case 6:
#line 301 "dhcp4_parser.yy"
                 { ctx.ctx_ = ctx.DHCP4; }
#line 793 "dhcp4_parser.cc"
    break;

  case 8:
#line 302 "dhcp4_parser.yy"
                       { ctx.ctx_ = ctx.INTERFACES_CONFIG; }
#line 799 "dhcp4_parser.cc"
    break;

  case 10:
#line 303 "dhcp4_parser.yy"
                   { ctx.ctx_ = ctx.SUBNET4; }
#line 805 "dhcp4_parser.cc"
    break;

  case 12:
#line 304 "dhcp4_parser.yy"
                 { ctx.ctx_ = ctx.POOLS; }
#line 811 "dhcp4_parser.cc"
    break;

  case 14:
#line 305 "dhcp4_parser.yy"
                       { ctx.ctx_ = ctx.RESERVATIONS; }
#line 817 "dhcp4_parser.cc"
    break;

  case 16:
#line 306 "dhcp4_parser.yy"
                       { ctx.ctx_ = ctx.DHCP4; }
#line 823 "dhcp4_parser.cc"
    break;

  case 18:
#line 307 "dhcp4_parser.yy"
                      { ctx.ctx_ = ctx.OPTION_DEF; }
#line 829 "dhcp4_parser.cc"
    break;

  case 20:
#line 308 "dhcp4_parser.yy"
                       { ctx.ctx_ = ctx.OPTION_DATA; }
#line 835 "dhcp4_parser.cc"
    break;

  case 22:
#line 309 "dhcp4_parser.yy"
                         { ctx.ctx_ = ctx.HOOKS_LIBRARIES; }
#line 841 "dhcp4_parser.cc"
    break;

  case 24:
#line 310 "dhcp4_parser.yy"
                     { ctx.ctx_ = ctx.DHCP_DDNS; }
#line 847 "dhcp4_parser.cc"
    break;

  case 26:
#line 311 "dhcp4_parser.yy"
                          { ctx.ctx_ = ctx.CONFIG_CONTROL; }
#line 853 "dhcp4_parser.cc"
    break;

  case 28:
#line 319 "dhcp4_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location))); }
#line 859 "dhcp4_parser.cc"
    break;

  case 29:
#line 320 "dhcp4_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location))); }
#line 865 "dhcp4_parser.cc"
    break;

  case 30:
#line 321 "dhcp4_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location))); }
#line 871 "dhcp4_parser.cc"
    break;

  case 31:
#line 322 "dhcp4_parser.yy"
              { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location))); }
#line 877 "dhcp4_parser.cc"
    break;

  case 32:
#line 323 "dhcp4_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new NullElement(ctx.loc2pos(yystack_[0].location))); }
#line 883 "dhcp4_parser.cc"
    break;

  case 33:
#line 324 "dhcp4_parser.yy"
            { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 889 "dhcp4_parser.cc"
    break;

  case 34:
#line 325 "dhcp4_parser.yy"
                    { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 895 "dhcp4_parser.cc"
    break;

  case 35:
#line 328 "dhcp4_parser.yy"
                {
    // Push back the JSON value on the stack
    ctx.stack_.push_back(yystack_[0].value.as < ElementPtr > ());
}
#line 904 "dhcp4_parser.cc"
    break;

  case 36:
#line 333 "dhcp4_parser.yy"
                     {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 915 "dhcp4_parser.cc"
    break;

  case 37:
#line 338 "dhcp4_parser.yy"
                             {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
}
#line 925 "dhcp4_parser.cc"
    break;

  case 38:
#line 344 "dhcp4_parser.yy"
                { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 931 "dhcp4_parser.cc"
    break;

  case 41:
#line 351 "dhcp4_parser.yy"
                                  {
                  // map containing a single entry
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 940 "dhcp4_parser.cc"
    break;

  case 42:
#line 355 "dhcp4_parser.yy"
                                                      {
                  // map consisting of a shorter map followed by
                  // comma and string:value
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 950 "dhcp4_parser.cc"
    break;

  case 43:
#line 362 "dhcp4_parser.yy"
                              {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(l);
}
#line 959 "dhcp4_parser.cc"
    break;

  case 44:
#line 365 "dhcp4_parser.yy"
                               {
    // list parsing complete. Put any sanity checking here
}
#line 967 "dhcp4_parser.cc"
    break;

  case 47:
#line 373 "dhcp4_parser.yy"
                      {
                  // List consisting of a single element.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 976 "dhcp4_parser.cc"
    break;

  case 48:
#line 377 "dhcp4_parser.yy"
                                           {
                  // List ending with , and a value.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 985 "dhcp4_parser.cc"
    break;

  case 49:
#line 384 "dhcp4_parser.yy"
                              {
    // List parsing about to start
}
#line 993 "dhcp4_parser.cc"
    break;

  case 50:
#line 386 "dhcp4_parser.yy"
                                       {
    // list parsing complete. Put any sanity checking here
    //ctx.stack_.pop_back();
}
#line 1002 "dhcp4_parser.cc"
    break;

  case 53:
#line 395 "dhcp4_parser.yy"
                               {
                          ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
                          ctx.stack_.back()->add(s);
                          }
#line 1011 "dhcp4_parser.cc"
    break;

  case 54:
#line 399 "dhcp4_parser.yy"
                                                            {
                          ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
                          ctx.stack_.back()->add(s);
                          }
#line 1020 "dhcp4_parser.cc"
    break;

  case 55:
#line 410 "dhcp4_parser.yy"
                                {
    const std::string& where = ctx.contextName();
    const std::string& keyword = yystack_[1].value.as < std::string > ();
    error(yystack_[1].location,
          "got unexpected keyword \"" + keyword + "\" in " + where + " map.");
}
#line 1031 "dhcp4_parser.cc"
    break;

  case 56:
#line 419 "dhcp4_parser.yy"
                           {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 1042 "dhcp4_parser.cc"
    break;

  case 57:
#line 424 "dhcp4_parser.yy"
                                {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.

    // Dhcp4 is required
    ctx.require("Dhcp4", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
}
#line 1055 "dhcp4_parser.cc"
    break;

  case 61:
#line 442 "dhcp4_parser.yy"
                    {
    // This code is executed when we're about to start parsing
    // the content of the map
    // Prevent against duplicate.
    ctx.unique("Dhcp4", ctx.loc2pos(yystack_[0].location));
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("Dhcp4", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.DHCP4);
}
#line 1070 "dhcp4_parser.cc"
    break;

  case 62:
#line 451 "dhcp4_parser.yy"
                                                    {
    // No global parameter is required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1080 "dhcp4_parser.cc"
    break;

  case 63:
#line 459 "dhcp4_parser.yy"
                          {
    // Parse the Dhcp4 map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 1090 "dhcp4_parser.cc"
    break;

  case 64:
#line 463 "dhcp4_parser.yy"
                               {
    // No global parameter is required
    // parsing completed
}
#line 1099 "dhcp4_parser.cc"
    break;

  case 128:
#line 537 "dhcp4_parser.yy"
                                             {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("valid-lifetime", prf);
}
#line 1108 "dhcp4_parser.cc"
    break;

  case 129:
#line 542 "dhcp4_parser.yy"
                                                     {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("min-valid-lifetime", prf);
}
#line 1117 "dhcp4_parser.cc"
    break;

  case 130:
#line 547 "dhcp4_parser.yy"
                                                     {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-valid-lifetime", prf);
}
#line 1126 "dhcp4_parser.cc"
    break;

  case 131:
#line 552 "dhcp4_parser.yy"
                                       {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("renew-timer", prf);
}
#line 1135 "dhcp4_parser.cc"
    break;

  case 132:
#line 557 "dhcp4_parser.yy"
                                         {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("rebind-timer", prf);
}
#line 1144 "dhcp4_parser.cc"
    break;

  case 133:
#line 562 "dhcp4_parser.yy"
                                                       {
    ElementPtr ctt(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("calculate-tee-times", ctt);
}
#line 1153 "dhcp4_parser.cc"
    break;

  case 134:
#line 567 "dhcp4_parser.yy"
                                   {
    ElementPtr t1(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("t1-percent", t1);
}
#line 1162 "dhcp4_parser.cc"
    break;

  case 135:
#line 572 "dhcp4_parser.yy"
                                   {
    ElementPtr t2(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("t2-percent", t2);
}
#line 1171 "dhcp4_parser.cc"
    break;

  case 136:
#line 577 "dhcp4_parser.yy"
                                                                 {
    ElementPtr dpp(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("decline-probation-period", dpp);
}
#line 1180 "dhcp4_parser.cc"
    break;

  case 137:
#line 582 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1188 "dhcp4_parser.cc"
    break;

  case 138:
#line 584 "dhcp4_parser.yy"
               {
    ElementPtr stag(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-tag", stag);
    ctx.leave();
}
#line 1198 "dhcp4_parser.cc"
    break;

  case 139:
#line 590 "dhcp4_parser.yy"
                                             {
    ElementPtr echo(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("echo-client-id", echo);
}
#line 1207 "dhcp4_parser.cc"
    break;

  case 140:
#line 595 "dhcp4_parser.yy"
                                               {
    ElementPtr match(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("match-client-id", match);
}
#line 1216 "dhcp4_parser.cc"
    break;

  case 141:
#line 600 "dhcp4_parser.yy"
                                           {
    ElementPtr prf(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("authoritative", prf);
}
#line 1225 "dhcp4_parser.cc"
    break;

  case 142:
#line 605 "dhcp4_parser.yy"
                                                   {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-send-updates", b);
}
#line 1234 "dhcp4_parser.cc"
    break;

  case 143:
#line 610 "dhcp4_parser.yy"
                                                               {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-override-no-update", b);
}
#line 1243 "dhcp4_parser.cc"
    break;

  case 144:
#line 615 "dhcp4_parser.yy"
                                                                       {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-override-client-update", b);
}
#line 1252 "dhcp4_parser.cc"
    break;

  case 145:
#line 620 "dhcp4_parser.yy"
                                                   {
    ctx.enter(ctx.REPLACE_CLIENT_NAME);
}
#line 1260 "dhcp4_parser.cc"
    break;

  case 146:
#line 622 "dhcp4_parser.yy"
                                       {
    ctx.stack_.back()->set("ddns-replace-client-name", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1269 "dhcp4_parser.cc"
    break;

  case 147:
#line 628 "dhcp4_parser.yy"
                 {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("when-present", ctx.loc2pos(yystack_[0].location)));
      }
#line 1277 "dhcp4_parser.cc"
    break;

  case 148:
#line 631 "dhcp4_parser.yy"
          {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("never", ctx.loc2pos(yystack_[0].location)));
      }
#line 1285 "dhcp4_parser.cc"
    break;

  case 149:
#line 634 "dhcp4_parser.yy"
           {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("always", ctx.loc2pos(yystack_[0].location)));
      }
#line 1293 "dhcp4_parser.cc"
    break;

  case 150:
#line 637 "dhcp4_parser.yy"
                     {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("when-not-present", ctx.loc2pos(yystack_[0].location)));
      }
#line 1301 "dhcp4_parser.cc"
    break;

  case 151:
#line 640 "dhcp4_parser.yy"
            {
      error(yystack_[0].location, "boolean values for the replace-client-name are "
                "no longer supported");
      }
#line 1310 "dhcp4_parser.cc"
    break;

  case 152:
#line 646 "dhcp4_parser.yy"
                                             {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1318 "dhcp4_parser.cc"
    break;

  case 153:
#line 648 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-generated-prefix", s);
    ctx.leave();
}
#line 1328 "dhcp4_parser.cc"
    break;

  case 154:
#line 654 "dhcp4_parser.yy"
                                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1336 "dhcp4_parser.cc"
    break;

  case 155:
#line 656 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-qualifying-suffix", s);
    ctx.leave();
}
#line 1346 "dhcp4_parser.cc"
    break;

  case 156:
#line 662 "dhcp4_parser.yy"
                                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1354 "dhcp4_parser.cc"
    break;

  case 157:
#line 664 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-set", s);
    ctx.leave();
}
#line 1364 "dhcp4_parser.cc"
    break;

  case 158:
#line 670 "dhcp4_parser.yy"
                                                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1372 "dhcp4_parser.cc"
    break;

  case 159:
#line 672 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-replacement", s);
    ctx.leave();
}
#line 1382 "dhcp4_parser.cc"
    break;

  case 160:
#line 678 "dhcp4_parser.yy"
                                                       {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("store-extended-info", b);
}
#line 1391 "dhcp4_parser.cc"
    break;

  case 161:
#line 683 "dhcp4_parser.yy"
                                                                             {
    ElementPtr count(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("statistic-default-sample-count", count);
}
#line 1400 "dhcp4_parser.cc"
    break;

  case 162:
#line 688 "dhcp4_parser.yy"
                                                                         {
    ElementPtr age(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("statistic-default-sample-age", age);
}
#line 1409 "dhcp4_parser.cc"
    break;

  case 163:
#line 693 "dhcp4_parser.yy"
                                     {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interfaces-config", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.INTERFACES_CONFIG);
}
#line 1420 "dhcp4_parser.cc"
    break;

  case 164:
#line 698 "dhcp4_parser.yy"
                                                               {
    // No interfaces config param is required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1430 "dhcp4_parser.cc"
    break;

  case 174:
#line 717 "dhcp4_parser.yy"
                                {
    // Parse the interfaces-config map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 1440 "dhcp4_parser.cc"
    break;

  case 175:
#line 721 "dhcp4_parser.yy"
                                          {
    // No interfaces config param is required
    // parsing completed
}
#line 1449 "dhcp4_parser.cc"
    break;

  case 176:
#line 726 "dhcp4_parser.yy"
                            {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interfaces", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1460 "dhcp4_parser.cc"
    break;

  case 177:
#line 731 "dhcp4_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1469 "dhcp4_parser.cc"
    break;

  case 178:
#line 736 "dhcp4_parser.yy"
                                   {
    ctx.enter(ctx.DHCP_SOCKET_TYPE);
}
#line 1477 "dhcp4_parser.cc"
    break;

  case 179:
#line 738 "dhcp4_parser.yy"
                    {
    ctx.stack_.back()->set("dhcp-socket-type", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1486 "dhcp4_parser.cc"
    break;

  case 180:
#line 743 "dhcp4_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("raw", ctx.loc2pos(yystack_[0].location))); }
#line 1492 "dhcp4_parser.cc"
    break;

  case 181:
#line 744 "dhcp4_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("udp", ctx.loc2pos(yystack_[0].location))); }
#line 1498 "dhcp4_parser.cc"
    break;

  case 182:
#line 747 "dhcp4_parser.yy"
                                       {
    ctx.enter(ctx.OUTBOUND_INTERFACE);
}
#line 1506 "dhcp4_parser.cc"
    break;

  case 183:
#line 749 "dhcp4_parser.yy"
                                 {
    ctx.stack_.back()->set("outbound-interface", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1515 "dhcp4_parser.cc"
    break;

  case 184:
#line 754 "dhcp4_parser.yy"
                                          {
    yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("same-as-inbound", ctx.loc2pos(yystack_[0].location)));
}
#line 1523 "dhcp4_parser.cc"
    break;

  case 185:
#line 756 "dhcp4_parser.yy"
                {
    yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("use-routing", ctx.loc2pos(yystack_[0].location)));
    }
#line 1531 "dhcp4_parser.cc"
    break;

  case 186:
#line 760 "dhcp4_parser.yy"
                                   {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("re-detect", b);
}
#line 1540 "dhcp4_parser.cc"
    break;

  case 187:
#line 766 "dhcp4_parser.yy"
                               {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("lease-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.LEASE_DATABASE);
}
#line 1551 "dhcp4_parser.cc"
    break;

  case 188:
#line 771 "dhcp4_parser.yy"
                                                          {
    // The type parameter is required
    ctx.require("type", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1562 "dhcp4_parser.cc"
    break;

  case 189:
#line 778 "dhcp4_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("sanity-checks", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.SANITY_CHECKS);
}
#line 1573 "dhcp4_parser.cc"
    break;

  case 190:
#line 783 "dhcp4_parser.yy"
                                                           {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1582 "dhcp4_parser.cc"
    break;

  case 194:
#line 793 "dhcp4_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1590 "dhcp4_parser.cc"
    break;

  case 195:
#line 795 "dhcp4_parser.yy"
               {

    if ( (string(yystack_[0].value.as < std::string > ()) == "none") ||
         (string(yystack_[0].value.as < std::string > ()) == "warn") ||
         (string(yystack_[0].value.as < std::string > ()) == "fix") ||
         (string(yystack_[0].value.as < std::string > ()) == "fix-del") ||
         (string(yystack_[0].value.as < std::string > ()) == "del")) {
        ElementPtr user(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
        ctx.stack_.back()->set("lease-checks", user);
        ctx.leave();
    } else {
        error(yystack_[0].location, "Unsupported 'lease-checks value: " + string(yystack_[0].value.as < std::string > ()) +
              ", supported values are: none, warn, fix, fix-del, del");
    }
}
#line 1610 "dhcp4_parser.cc"
    break;

  case 196:
#line 811 "dhcp4_parser.yy"
                               {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hosts-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.HOSTS_DATABASE);
}
#line 1621 "dhcp4_parser.cc"
    break;

  case 197:
#line 816 "dhcp4_parser.yy"
                                                          {
    // The type parameter is required
    ctx.require("type", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1632 "dhcp4_parser.cc"
    break;

  case 198:
#line 823 "dhcp4_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hosts-databases", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOSTS_DATABASE);
}
#line 1643 "dhcp4_parser.cc"
    break;

  case 199:
#line 828 "dhcp4_parser.yy"
                                                      {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1652 "dhcp4_parser.cc"
    break;

  case 204:
#line 841 "dhcp4_parser.yy"
                         {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1662 "dhcp4_parser.cc"
    break;

  case 205:
#line 845 "dhcp4_parser.yy"
                                     {
    // The type parameter is required
    ctx.require("type", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 1672 "dhcp4_parser.cc"
    break;

  case 232:
#line 881 "dhcp4_parser.yy"
                    {
    ctx.enter(ctx.DATABASE_TYPE);
}
#line 1680 "dhcp4_parser.cc"
    break;

  case 233:
#line 883 "dhcp4_parser.yy"
                {
    ctx.stack_.back()->set("type", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1689 "dhcp4_parser.cc"
    break;

  case 234:
#line 888 "dhcp4_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("memfile", ctx.loc2pos(yystack_[0].location))); }
#line 1695 "dhcp4_parser.cc"
    break;

  case 235:
#line 889 "dhcp4_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("mysql", ctx.loc2pos(yystack_[0].location))); }
#line 1701 "dhcp4_parser.cc"
    break;

  case 236:
#line 890 "dhcp4_parser.yy"
                    { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("postgresql", ctx.loc2pos(yystack_[0].location))); }
#line 1707 "dhcp4_parser.cc"
    break;

  case 237:
#line 891 "dhcp4_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("cql", ctx.loc2pos(yystack_[0].location))); }
#line 1713 "dhcp4_parser.cc"
    break;

  case 238:
#line 894 "dhcp4_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1721 "dhcp4_parser.cc"
    break;

  case 239:
#line 896 "dhcp4_parser.yy"
               {
    ElementPtr user(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("user", user);
    ctx.leave();
}
#line 1731 "dhcp4_parser.cc"
    break;

  case 240:
#line 902 "dhcp4_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1739 "dhcp4_parser.cc"
    break;

  case 241:
#line 904 "dhcp4_parser.yy"
               {
    ElementPtr pwd(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("password", pwd);
    ctx.leave();
}
#line 1749 "dhcp4_parser.cc"
    break;

  case 242:
#line 910 "dhcp4_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1757 "dhcp4_parser.cc"
    break;

  case 243:
#line 912 "dhcp4_parser.yy"
               {
    ElementPtr h(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("host", h);
    ctx.leave();
}
#line 1767 "dhcp4_parser.cc"
    break;

  case 244:
#line 918 "dhcp4_parser.yy"
                         {
    ElementPtr p(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("port", p);
}
#line 1776 "dhcp4_parser.cc"
    break;

  case 245:
#line 923 "dhcp4_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1784 "dhcp4_parser.cc"
    break;

  case 246:
#line 925 "dhcp4_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("name", name);
    ctx.leave();
}
#line 1794 "dhcp4_parser.cc"
    break;

  case 247:
#line 931 "dhcp4_parser.yy"
                               {
    ElementPtr n(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("persist", n);
}
#line 1803 "dhcp4_parser.cc"
    break;

  case 248:
#line 936 "dhcp4_parser.yy"
                                         {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("lfc-interval", n);
}
#line 1812 "dhcp4_parser.cc"
    break;

  case 249:
#line 941 "dhcp4_parser.yy"
                                 {
    ElementPtr n(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("readonly", n);
}
#line 1821 "dhcp4_parser.cc"
    break;

  case 250:
#line 946 "dhcp4_parser.yy"
                                               {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("connect-timeout", n);
}
#line 1830 "dhcp4_parser.cc"
    break;

  case 251:
#line 951 "dhcp4_parser.yy"
                                               {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("request-timeout", n);
}
#line 1839 "dhcp4_parser.cc"
    break;

  case 252:
#line 956 "dhcp4_parser.yy"
                                           {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("tcp-keepalive", n);
}
#line 1848 "dhcp4_parser.cc"
    break;

  case 253:
#line 961 "dhcp4_parser.yy"
                                       {
    ElementPtr n(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("tcp-nodelay", n);
}
#line 1857 "dhcp4_parser.cc"
    break;

  case 254:
#line 966 "dhcp4_parser.yy"
                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1865 "dhcp4_parser.cc"
    break;

  case 255:
#line 968 "dhcp4_parser.yy"
               {
    ElementPtr cp(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("contact-points", cp);
    ctx.leave();
}
#line 1875 "dhcp4_parser.cc"
    break;

  case 256:
#line 974 "dhcp4_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1883 "dhcp4_parser.cc"
    break;

  case 257:
#line 976 "dhcp4_parser.yy"
               {
    ElementPtr ks(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("keyspace", ks);
    ctx.leave();
}
#line 1893 "dhcp4_parser.cc"
    break;

  case 258:
#line 982 "dhcp4_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1901 "dhcp4_parser.cc"
    break;

  case 259:
#line 984 "dhcp4_parser.yy"
               {
    ElementPtr c(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("consistency", c);
    ctx.leave();
}
#line 1911 "dhcp4_parser.cc"
    break;

  case 260:
#line 990 "dhcp4_parser.yy"
                                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1919 "dhcp4_parser.cc"
    break;

  case 261:
#line 992 "dhcp4_parser.yy"
               {
    ElementPtr c(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("serial-consistency", c);
    ctx.leave();
}
#line 1929 "dhcp4_parser.cc"
    break;

  case 262:
#line 998 "dhcp4_parser.yy"
                                                       {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-reconnect-tries", n);
}
#line 1938 "dhcp4_parser.cc"
    break;

  case 263:
#line 1003 "dhcp4_parser.yy"
                                                       {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("reconnect-wait-time", n);
}
#line 1947 "dhcp4_parser.cc"
    break;

  case 264:
#line 1008 "dhcp4_parser.yy"
                                             {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-row-errors", n);
}
#line 1956 "dhcp4_parser.cc"
    break;

  case 265:
#line 1014 "dhcp4_parser.yy"
                                                           {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("host-reservation-identifiers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOST_RESERVATION_IDENTIFIERS);
}
#line 1967 "dhcp4_parser.cc"
    break;

  case 266:
#line 1019 "dhcp4_parser.yy"
                                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1976 "dhcp4_parser.cc"
    break;

  case 274:
#line 1035 "dhcp4_parser.yy"
               {
    ElementPtr duid(new StringElement("duid", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(duid);
}
#line 1985 "dhcp4_parser.cc"
    break;

  case 275:
#line 1040 "dhcp4_parser.yy"
                           {
    ElementPtr hwaddr(new StringElement("hw-address", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(hwaddr);
}
#line 1994 "dhcp4_parser.cc"
    break;

  case 276:
#line 1045 "dhcp4_parser.yy"
                        {
    ElementPtr circuit(new StringElement("circuit-id", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(circuit);
}
#line 2003 "dhcp4_parser.cc"
    break;

  case 277:
#line 1050 "dhcp4_parser.yy"
                      {
    ElementPtr client(new StringElement("client-id", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(client);
}
#line 2012 "dhcp4_parser.cc"
    break;

  case 278:
#line 1055 "dhcp4_parser.yy"
                 {
    ElementPtr flex_id(new StringElement("flex-id", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(flex_id);
}
#line 2021 "dhcp4_parser.cc"
    break;

  case 279:
#line 1062 "dhcp4_parser.yy"
                                           {
    ElementPtr mt(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("multi-threading", mt);
    ctx.stack_.push_back(mt);
    ctx.enter(ctx.DHCP_MULTI_THREADING);
}
#line 2032 "dhcp4_parser.cc"
    break;

  case 280:
#line 1067 "dhcp4_parser.yy"
                                                             {
    // The enable parameter is required.
    ctx.require("enable-multi-threading", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2043 "dhcp4_parser.cc"
    break;

  case 289:
#line 1086 "dhcp4_parser.yy"
                                                             {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enable-multi-threading", b);
}
#line 2052 "dhcp4_parser.cc"
    break;

  case 290:
#line 1091 "dhcp4_parser.yy"
                                                 {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("thread-pool-size", prf);
}
#line 2061 "dhcp4_parser.cc"
    break;

  case 291:
#line 1096 "dhcp4_parser.yy"
                                                   {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("packet-queue-size", prf);
}
#line 2070 "dhcp4_parser.cc"
    break;

  case 292:
#line 1101 "dhcp4_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hooks-libraries", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOOKS_LIBRARIES);
}
#line 2081 "dhcp4_parser.cc"
    break;

  case 293:
#line 1106 "dhcp4_parser.yy"
                                                             {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2090 "dhcp4_parser.cc"
    break;

  case 298:
#line 1119 "dhcp4_parser.yy"
                              {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2100 "dhcp4_parser.cc"
    break;

  case 299:
#line 1123 "dhcp4_parser.yy"
                              {
    // The library hooks parameter is required
    ctx.require("library", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2110 "dhcp4_parser.cc"
    break;

  case 300:
#line 1129 "dhcp4_parser.yy"
                                  {
    // Parse the hooks-libraries list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2120 "dhcp4_parser.cc"
    break;

  case 301:
#line 1133 "dhcp4_parser.yy"
                              {
    // The library hooks parameter is required
    ctx.require("library", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2130 "dhcp4_parser.cc"
    break;

  case 309:
#line 1150 "dhcp4_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2138 "dhcp4_parser.cc"
    break;

  case 310:
#line 1152 "dhcp4_parser.yy"
               {
    ElementPtr lib(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("library", lib);
    ctx.leave();
}
#line 2148 "dhcp4_parser.cc"
    break;

  case 311:
#line 1158 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2156 "dhcp4_parser.cc"
    break;

  case 312:
#line 1160 "dhcp4_parser.yy"
                  {
    ctx.stack_.back()->set("parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 2165 "dhcp4_parser.cc"
    break;

  case 313:
#line 1166 "dhcp4_parser.yy"
                                                     {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("expired-leases-processing", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.EXPIRED_LEASES_PROCESSING);
}
#line 2176 "dhcp4_parser.cc"
    break;

  case 314:
#line 1171 "dhcp4_parser.yy"
                                                            {
    // No expired lease parameter is required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2186 "dhcp4_parser.cc"
    break;

  case 323:
#line 1189 "dhcp4_parser.yy"
                                                               {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("reclaim-timer-wait-time", value);
}
#line 2195 "dhcp4_parser.cc"
    break;

  case 324:
#line 1194 "dhcp4_parser.yy"
                                                                               {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flush-reclaimed-timer-wait-time", value);
}
#line 2204 "dhcp4_parser.cc"
    break;

  case 325:
#line 1199 "dhcp4_parser.yy"
                                                       {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hold-reclaimed-time", value);
}
#line 2213 "dhcp4_parser.cc"
    break;

  case 326:
#line 1204 "dhcp4_parser.yy"
                                                     {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-reclaim-leases", value);
}
#line 2222 "dhcp4_parser.cc"
    break;

  case 327:
#line 1209 "dhcp4_parser.yy"
                                                 {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-reclaim-time", value);
}
#line 2231 "dhcp4_parser.cc"
    break;

  case 328:
#line 1214 "dhcp4_parser.yy"
                                                               {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("unwarned-reclaim-cycles", value);
}
#line 2240 "dhcp4_parser.cc"
    break;

  case 329:
#line 1222 "dhcp4_parser.yy"
                      {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("subnet4", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.SUBNET4);
}
#line 2251 "dhcp4_parser.cc"
    break;

  case 330:
#line 1227 "dhcp4_parser.yy"
                                                             {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2260 "dhcp4_parser.cc"
    break;

  case 335:
#line 1247 "dhcp4_parser.yy"
                        {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2270 "dhcp4_parser.cc"
    break;

  case 336:
#line 1251 "dhcp4_parser.yy"
                                {
    // Once we reached this place, the subnet parsing is now complete.
    // If we want to, we can implement default values here.
    // In particular we can do things like this:
    // if (!ctx.stack_.back()->get("interface")) {
    //     ctx.stack_.back()->set("interface", StringElement("loopback"));
    // }
    //
    // We can also stack up one level (Dhcp4) and copy over whatever
    // global parameters we want to:
    // if (!ctx.stack_.back()->get("renew-timer")) {
    //     ElementPtr renew = ctx_stack_[...].get("renew-timer");
    //     if (renew) {
    //         ctx.stack_.back()->set("renew-timer", renew);
    //     }
    // }

    // The subnet subnet4 parameter is required
    ctx.require("subnet", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2296 "dhcp4_parser.cc"
    break;

  case 337:
#line 1273 "dhcp4_parser.yy"
                            {
    // Parse the subnet4 list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2306 "dhcp4_parser.cc"
    break;

  case 338:
#line 1277 "dhcp4_parser.yy"
                                {
    // The subnet subnet4 parameter is required
    ctx.require("subnet", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2316 "dhcp4_parser.cc"
    break;

  case 383:
#line 1333 "dhcp4_parser.yy"
               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2324 "dhcp4_parser.cc"
    break;

  case 384:
#line 1335 "dhcp4_parser.yy"
               {
    ElementPtr subnet(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("subnet", subnet);
    ctx.leave();
}
#line 2334 "dhcp4_parser.cc"
    break;

  case 385:
#line 1341 "dhcp4_parser.yy"
                                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2342 "dhcp4_parser.cc"
    break;

  case 386:
#line 1343 "dhcp4_parser.yy"
               {
    ElementPtr iface(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("4o6-interface", iface);
    ctx.leave();
}
#line 2352 "dhcp4_parser.cc"
    break;

  case 387:
#line 1349 "dhcp4_parser.yy"
                                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2360 "dhcp4_parser.cc"
    break;

  case 388:
#line 1351 "dhcp4_parser.yy"
               {
    ElementPtr iface(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("4o6-interface-id", iface);
    ctx.leave();
}
#line 2370 "dhcp4_parser.cc"
    break;

  case 389:
#line 1357 "dhcp4_parser.yy"
                                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2378 "dhcp4_parser.cc"
    break;

  case 390:
#line 1359 "dhcp4_parser.yy"
               {
    ElementPtr iface(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("4o6-subnet", iface);
    ctx.leave();
}
#line 2388 "dhcp4_parser.cc"
    break;

  case 391:
#line 1365 "dhcp4_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2396 "dhcp4_parser.cc"
    break;

  case 392:
#line 1367 "dhcp4_parser.yy"
               {
    ElementPtr iface(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interface", iface);
    ctx.leave();
}
#line 2406 "dhcp4_parser.cc"
    break;

  case 393:
#line 1373 "dhcp4_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2414 "dhcp4_parser.cc"
    break;

  case 394:
#line 1375 "dhcp4_parser.yy"
               {
    ElementPtr cls(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-class", cls);
    ctx.leave();
}
#line 2424 "dhcp4_parser.cc"
    break;

  case 395:
#line 1381 "dhcp4_parser.yy"
                                               {
    ElementPtr c(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("require-client-classes", c);
    ctx.stack_.push_back(c);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2435 "dhcp4_parser.cc"
    break;

  case 396:
#line 1386 "dhcp4_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2444 "dhcp4_parser.cc"
    break;

  case 397:
#line 1391 "dhcp4_parser.yy"
                                   {
    ctx.enter(ctx.RESERVATION_MODE);
}
#line 2452 "dhcp4_parser.cc"
    break;

  case 398:
#line 1393 "dhcp4_parser.yy"
                {
    ctx.stack_.back()->set("reservation-mode", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 2461 "dhcp4_parser.cc"
    break;

  case 399:
#line 1398 "dhcp4_parser.yy"
                  { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("disabled", ctx.loc2pos(yystack_[0].location))); }
#line 2467 "dhcp4_parser.cc"
    break;

  case 400:
#line 1399 "dhcp4_parser.yy"
                     { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("out-of-pool", ctx.loc2pos(yystack_[0].location))); }
#line 2473 "dhcp4_parser.cc"
    break;

  case 401:
#line 1400 "dhcp4_parser.yy"
                { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("global", ctx.loc2pos(yystack_[0].location))); }
#line 2479 "dhcp4_parser.cc"
    break;

  case 402:
#line 1401 "dhcp4_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("all", ctx.loc2pos(yystack_[0].location))); }
#line 2485 "dhcp4_parser.cc"
    break;

  case 403:
#line 1404 "dhcp4_parser.yy"
                     {
    ElementPtr id(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("id", id);
}
#line 2494 "dhcp4_parser.cc"
    break;

  case 404:
#line 1411 "dhcp4_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("shared-networks", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.SHARED_NETWORK);
}
#line 2505 "dhcp4_parser.cc"
    break;

  case 405:
#line 1416 "dhcp4_parser.yy"
                                                                {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2514 "dhcp4_parser.cc"
    break;

  case 410:
#line 1431 "dhcp4_parser.yy"
                               {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2524 "dhcp4_parser.cc"
    break;

  case 411:
#line 1435 "dhcp4_parser.yy"
                                       {
    ctx.stack_.pop_back();
}
#line 2532 "dhcp4_parser.cc"
    break;

  case 447:
#line 1482 "dhcp4_parser.yy"
                            {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-def", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OPTION_DEF);
}
#line 2543 "dhcp4_parser.cc"
    break;

  case 448:
#line 1487 "dhcp4_parser.yy"
                                                                {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2552 "dhcp4_parser.cc"
    break;

  case 449:
#line 1495 "dhcp4_parser.yy"
                                    {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2561 "dhcp4_parser.cc"
    break;

  case 450:
#line 1498 "dhcp4_parser.yy"
                                 {
    // parsing completed
}
#line 2569 "dhcp4_parser.cc"
    break;

  case 455:
#line 1514 "dhcp4_parser.yy"
                                 {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2579 "dhcp4_parser.cc"
    break;

  case 456:
#line 1518 "dhcp4_parser.yy"
                                   {
    // The name, code and type option def parameters are required.
    ctx.require("name", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("code", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("type", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2591 "dhcp4_parser.cc"
    break;

  case 457:
#line 1529 "dhcp4_parser.yy"
                               {
    // Parse the option-def list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2601 "dhcp4_parser.cc"
    break;

  case 458:
#line 1533 "dhcp4_parser.yy"
                                   {
    // The name, code and type option def parameters are required.
    ctx.require("name", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("code", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("type", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2613 "dhcp4_parser.cc"
    break;

  case 474:
#line 1565 "dhcp4_parser.yy"
                         {
    ElementPtr code(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("code", code);
}
#line 2622 "dhcp4_parser.cc"
    break;

  case 476:
#line 1572 "dhcp4_parser.yy"
                      {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2630 "dhcp4_parser.cc"
    break;

  case 477:
#line 1574 "dhcp4_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("type", prf);
    ctx.leave();
}
#line 2640 "dhcp4_parser.cc"
    break;

  case 478:
#line 1580 "dhcp4_parser.yy"
                                      {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2648 "dhcp4_parser.cc"
    break;

  case 479:
#line 1582 "dhcp4_parser.yy"
               {
    ElementPtr rtypes(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("record-types", rtypes);
    ctx.leave();
}
#line 2658 "dhcp4_parser.cc"
    break;

  case 480:
#line 1588 "dhcp4_parser.yy"
             {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2666 "dhcp4_parser.cc"
    break;

  case 481:
#line 1590 "dhcp4_parser.yy"
               {
    ElementPtr space(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("space", space);
    ctx.leave();
}
#line 2676 "dhcp4_parser.cc"
    break;

  case 483:
#line 1598 "dhcp4_parser.yy"
                                    {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2684 "dhcp4_parser.cc"
    break;

  case 484:
#line 1600 "dhcp4_parser.yy"
               {
    ElementPtr encap(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("encapsulate", encap);
    ctx.leave();
}
#line 2694 "dhcp4_parser.cc"
    break;

  case 485:
#line 1606 "dhcp4_parser.yy"
                                      {
    ElementPtr array(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("array", array);
}
#line 2703 "dhcp4_parser.cc"
    break;

  case 486:
#line 1615 "dhcp4_parser.yy"
                              {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-data", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OPTION_DATA);
}
#line 2714 "dhcp4_parser.cc"
    break;

  case 487:
#line 1620 "dhcp4_parser.yy"
                                                                 {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2723 "dhcp4_parser.cc"
    break;

  case 492:
#line 1639 "dhcp4_parser.yy"
                                  {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2733 "dhcp4_parser.cc"
    break;

  case 493:
#line 1643 "dhcp4_parser.yy"
                                    {
    /// @todo: the code or name parameters are required.
    ctx.stack_.pop_back();
}
#line 2742 "dhcp4_parser.cc"
    break;

  case 494:
#line 1651 "dhcp4_parser.yy"
                                {
    // Parse the option-data list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2752 "dhcp4_parser.cc"
    break;

  case 495:
#line 1655 "dhcp4_parser.yy"
                                    {
    /// @todo: the code or name parameters are required.
    // parsing completed
}
#line 2761 "dhcp4_parser.cc"
    break;

  case 510:
#line 1688 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2769 "dhcp4_parser.cc"
    break;

  case 511:
#line 1690 "dhcp4_parser.yy"
               {
    ElementPtr data(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("data", data);
    ctx.leave();
}
#line 2779 "dhcp4_parser.cc"
    break;

  case 514:
#line 1700 "dhcp4_parser.yy"
                                                 {
    ElementPtr space(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("csv-format", space);
}
#line 2788 "dhcp4_parser.cc"
    break;

  case 515:
#line 1705 "dhcp4_parser.yy"
                                                   {
    ElementPtr persist(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("always-send", persist);
}
#line 2797 "dhcp4_parser.cc"
    break;

  case 516:
#line 1713 "dhcp4_parser.yy"
                  {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pools", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.POOLS);
}
#line 2808 "dhcp4_parser.cc"
    break;

  case 517:
#line 1718 "dhcp4_parser.yy"
                                                           {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2817 "dhcp4_parser.cc"
    break;

  case 522:
#line 1733 "dhcp4_parser.yy"
                                {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2827 "dhcp4_parser.cc"
    break;

  case 523:
#line 1737 "dhcp4_parser.yy"
                             {
    // The pool parameter is required.
    ctx.require("pool", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2837 "dhcp4_parser.cc"
    break;

  case 524:
#line 1743 "dhcp4_parser.yy"
                          {
    // Parse the pool list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2847 "dhcp4_parser.cc"
    break;

  case 525:
#line 1747 "dhcp4_parser.yy"
                             {
    // The pool parameter is required.
    ctx.require("pool", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2857 "dhcp4_parser.cc"
    break;

  case 537:
#line 1768 "dhcp4_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2865 "dhcp4_parser.cc"
    break;

  case 538:
#line 1770 "dhcp4_parser.yy"
               {
    ElementPtr pool(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pool", pool);
    ctx.leave();
}
#line 2875 "dhcp4_parser.cc"
    break;

  case 539:
#line 1776 "dhcp4_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2883 "dhcp4_parser.cc"
    break;

  case 540:
#line 1778 "dhcp4_parser.yy"
                  {
    ElementPtr parent = ctx.stack_.back();
    ElementPtr user_context = yystack_[0].value.as < ElementPtr > ();
    ElementPtr old = parent->get("user-context");

    // Handle already existing user context
    if (old) {
        // Check if it was a comment or a duplicate
        if ((old->size() != 1) || !old->contains("comment")) {
            std::stringstream msg;
            msg << "duplicate user-context entries (previous at "
                << old->getPosition().str() << ")";
            error(yystack_[3].location, msg.str());
        }
        // Merge the comment
        user_context->set("comment", old->get("comment"));
    }

    // Set the user context
    parent->set("user-context", user_context);
    ctx.leave();
}
#line 2910 "dhcp4_parser.cc"
    break;

  case 541:
#line 1801 "dhcp4_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2918 "dhcp4_parser.cc"
    break;

  case 542:
#line 1803 "dhcp4_parser.yy"
               {
    ElementPtr parent = ctx.stack_.back();
    ElementPtr user_context(new MapElement(ctx.loc2pos(yystack_[3].location)));
    ElementPtr comment(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    user_context->set("comment", comment);

    // Handle already existing user context
    ElementPtr old = parent->get("user-context");
    if (old) {
        // Check for duplicate comment
        if (old->contains("comment")) {
            std::stringstream msg;
            msg << "duplicate user-context/comment entries (previous at "
                << old->getPosition().str() << ")";
            error(yystack_[3].location, msg.str());
        }
        // Merge the user context in the comment
        merge(user_context, old);
    }

    // Set the user context
    parent->set("user-context", user_context);
    ctx.leave();
}
#line 2947 "dhcp4_parser.cc"
    break;

  case 543:
#line 1831 "dhcp4_parser.yy"
                           {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("reservations", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.RESERVATIONS);
}
#line 2958 "dhcp4_parser.cc"
    break;

  case 544:
#line 1836 "dhcp4_parser.yy"
                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2967 "dhcp4_parser.cc"
    break;

  case 549:
#line 1849 "dhcp4_parser.yy"
                            {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2977 "dhcp4_parser.cc"
    break;

  case 550:
#line 1853 "dhcp4_parser.yy"
                                    {
    /// @todo: an identifier parameter is required.
    ctx.stack_.pop_back();
}
#line 2986 "dhcp4_parser.cc"
    break;

  case 551:
#line 1858 "dhcp4_parser.yy"
                                {
    // Parse the reservations list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2996 "dhcp4_parser.cc"
    break;

  case 552:
#line 1862 "dhcp4_parser.yy"
                                    {
    /// @todo: an identifier parameter is required.
    // parsing completed
}
#line 3005 "dhcp4_parser.cc"
    break;

  case 572:
#line 1893 "dhcp4_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3013 "dhcp4_parser.cc"
    break;

  case 573:
#line 1895 "dhcp4_parser.yy"
               {
    ElementPtr next_server(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("next-server", next_server);
    ctx.leave();
}
#line 3023 "dhcp4_parser.cc"
    break;

  case 574:
#line 1901 "dhcp4_parser.yy"
                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3031 "dhcp4_parser.cc"
    break;

  case 575:
#line 1903 "dhcp4_parser.yy"
               {
    ElementPtr srv(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-hostname", srv);
    ctx.leave();
}
#line 3041 "dhcp4_parser.cc"
    break;

  case 576:
#line 1909 "dhcp4_parser.yy"
                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3049 "dhcp4_parser.cc"
    break;

  case 577:
#line 1911 "dhcp4_parser.yy"
               {
    ElementPtr bootfile(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("boot-file-name", bootfile);
    ctx.leave();
}
#line 3059 "dhcp4_parser.cc"
    break;

  case 578:
#line 1917 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3067 "dhcp4_parser.cc"
    break;

  case 579:
#line 1919 "dhcp4_parser.yy"
               {
    ElementPtr addr(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ip-address", addr);
    ctx.leave();
}
#line 3077 "dhcp4_parser.cc"
    break;

  case 580:
#line 1925 "dhcp4_parser.yy"
                           {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ip-addresses", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3088 "dhcp4_parser.cc"
    break;

  case 581:
#line 1930 "dhcp4_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3097 "dhcp4_parser.cc"
    break;

  case 582:
#line 1935 "dhcp4_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3105 "dhcp4_parser.cc"
    break;

  case 583:
#line 1937 "dhcp4_parser.yy"
               {
    ElementPtr d(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("duid", d);
    ctx.leave();
}
#line 3115 "dhcp4_parser.cc"
    break;

  case 584:
#line 1943 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3123 "dhcp4_parser.cc"
    break;

  case 585:
#line 1945 "dhcp4_parser.yy"
               {
    ElementPtr hw(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hw-address", hw);
    ctx.leave();
}
#line 3133 "dhcp4_parser.cc"
    break;

  case 586:
#line 1951 "dhcp4_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3141 "dhcp4_parser.cc"
    break;

  case 587:
#line 1953 "dhcp4_parser.yy"
               {
    ElementPtr hw(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-id", hw);
    ctx.leave();
}
#line 3151 "dhcp4_parser.cc"
    break;

  case 588:
#line 1959 "dhcp4_parser.yy"
                             {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3159 "dhcp4_parser.cc"
    break;

  case 589:
#line 1961 "dhcp4_parser.yy"
               {
    ElementPtr hw(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("circuit-id", hw);
    ctx.leave();
}
#line 3169 "dhcp4_parser.cc"
    break;

  case 590:
#line 1967 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3177 "dhcp4_parser.cc"
    break;

  case 591:
#line 1969 "dhcp4_parser.yy"
               {
    ElementPtr hw(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flex-id", hw);
    ctx.leave();
}
#line 3187 "dhcp4_parser.cc"
    break;

  case 592:
#line 1975 "dhcp4_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3195 "dhcp4_parser.cc"
    break;

  case 593:
#line 1977 "dhcp4_parser.yy"
               {
    ElementPtr host(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname", host);
    ctx.leave();
}
#line 3205 "dhcp4_parser.cc"
    break;

  case 594:
#line 1983 "dhcp4_parser.yy"
                                           {
    ElementPtr c(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-classes", c);
    ctx.stack_.push_back(c);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3216 "dhcp4_parser.cc"
    break;

  case 595:
#line 1988 "dhcp4_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3225 "dhcp4_parser.cc"
    break;

  case 596:
#line 1996 "dhcp4_parser.yy"
             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("relay", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.RELAY);
}
#line 3236 "dhcp4_parser.cc"
    break;

  case 597:
#line 2001 "dhcp4_parser.yy"
                                                {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3245 "dhcp4_parser.cc"
    break;

  case 600:
#line 2013 "dhcp4_parser.yy"
                               {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-classes", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.CLIENT_CLASSES);
}
#line 3256 "dhcp4_parser.cc"
    break;

  case 601:
#line 2018 "dhcp4_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3265 "dhcp4_parser.cc"
    break;

  case 604:
#line 2027 "dhcp4_parser.yy"
                                   {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 3275 "dhcp4_parser.cc"
    break;

  case 605:
#line 2031 "dhcp4_parser.yy"
                                     {
    // The name client class parameter is required.
    ctx.require("name", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 3285 "dhcp4_parser.cc"
    break;

  case 622:
#line 2060 "dhcp4_parser.yy"
                        {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3293 "dhcp4_parser.cc"
    break;

  case 623:
#line 2062 "dhcp4_parser.yy"
               {
    ElementPtr test(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("test", test);
    ctx.leave();
}
#line 3303 "dhcp4_parser.cc"
    break;

  case 624:
#line 2068 "dhcp4_parser.yy"
                                                 {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("only-if-required", b);
}
#line 3312 "dhcp4_parser.cc"
    break;

  case 625:
#line 2075 "dhcp4_parser.yy"
                                         {
    ElementPtr time(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp4o6-port", time);
}
#line 3321 "dhcp4_parser.cc"
    break;

  case 626:
#line 2082 "dhcp4_parser.yy"
                               {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("control-socket", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKET);
}
#line 3332 "dhcp4_parser.cc"
    break;

  case 627:
#line 2087 "dhcp4_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3341 "dhcp4_parser.cc"
    break;

  case 643:
#line 2115 "dhcp4_parser.yy"
                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3349 "dhcp4_parser.cc"
    break;

  case 644:
#line 2117 "dhcp4_parser.yy"
               {
    ElementPtr stype(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-type", stype);
    ctx.leave();
}
#line 3359 "dhcp4_parser.cc"
    break;

  case 645:
#line 2123 "dhcp4_parser.yy"
                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3367 "dhcp4_parser.cc"
    break;

  case 646:
#line 2125 "dhcp4_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-name", name);
    ctx.leave();
}
#line 3377 "dhcp4_parser.cc"
    break;

  case 647:
#line 2131 "dhcp4_parser.yy"
                                                                                                             {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("on-boot-request-configuration-from-kea-netconf", b);
}
#line 3386 "dhcp4_parser.cc"
    break;

  case 648:
#line 2138 "dhcp4_parser.yy"
                                       {
    ElementPtr qc(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp-queue-control", qc);
    ctx.stack_.push_back(qc);
    ctx.enter(ctx.DHCP_QUEUE_CONTROL);
}
#line 3397 "dhcp4_parser.cc"
    break;

  case 649:
#line 2143 "dhcp4_parser.yy"
                                                           {
    // The enable queue parameter is required.
    ctx.require("enable-queue", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3408 "dhcp4_parser.cc"
    break;

  case 658:
#line 2162 "dhcp4_parser.yy"
                                         {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enable-queue", b);
}
#line 3417 "dhcp4_parser.cc"
    break;

  case 659:
#line 2167 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3425 "dhcp4_parser.cc"
    break;

  case 660:
#line 2169 "dhcp4_parser.yy"
               {
    ElementPtr qt(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("queue-type", qt);
    ctx.leave();
}
#line 3435 "dhcp4_parser.cc"
    break;

  case 661:
#line 2175 "dhcp4_parser.yy"
                                 {
    ElementPtr c(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("capacity", c);
}
#line 3444 "dhcp4_parser.cc"
    break;

  case 662:
#line 2180 "dhcp4_parser.yy"
                            {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3452 "dhcp4_parser.cc"
    break;

  case 663:
#line 2182 "dhcp4_parser.yy"
              {
    ctx.stack_.back()->set(yystack_[3].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3461 "dhcp4_parser.cc"
    break;

  case 664:
#line 2189 "dhcp4_parser.yy"
                     {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp-ddns", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.DHCP_DDNS);
}
#line 3472 "dhcp4_parser.cc"
    break;

  case 665:
#line 2194 "dhcp4_parser.yy"
                                                       {
    // The enable updates DHCP DDNS parameter is required.
    ctx.require("enable-updates", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3483 "dhcp4_parser.cc"
    break;

  case 666:
#line 2201 "dhcp4_parser.yy"
                              {
    // Parse the dhcp-ddns map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 3493 "dhcp4_parser.cc"
    break;

  case 667:
#line 2205 "dhcp4_parser.yy"
                                  {
    // The enable updates DHCP DDNS parameter is required.
    ctx.require("enable-updates", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 3503 "dhcp4_parser.cc"
    break;

  case 688:
#line 2235 "dhcp4_parser.yy"
                                             {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enable-updates", b);
}
#line 3512 "dhcp4_parser.cc"
    break;

  case 689:
#line 2240 "dhcp4_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3520 "dhcp4_parser.cc"
    break;

  case 690:
#line 2242 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-ip", s);
    ctx.leave();
}
#line 3530 "dhcp4_parser.cc"
    break;

  case 691:
#line 2248 "dhcp4_parser.yy"
                                       {
    ElementPtr i(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-port", i);
}
#line 3539 "dhcp4_parser.cc"
    break;

  case 692:
#line 2253 "dhcp4_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3547 "dhcp4_parser.cc"
    break;

  case 693:
#line 2255 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("sender-ip", s);
    ctx.leave();
}
#line 3557 "dhcp4_parser.cc"
    break;

  case 694:
#line 2261 "dhcp4_parser.yy"
                                       {
    ElementPtr i(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("sender-port", i);
}
#line 3566 "dhcp4_parser.cc"
    break;

  case 695:
#line 2266 "dhcp4_parser.yy"
                                             {
    ElementPtr i(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-queue-size", i);
}
#line 3575 "dhcp4_parser.cc"
    break;

  case 696:
#line 2271 "dhcp4_parser.yy"
                           {
    ctx.enter(ctx.NCR_PROTOCOL);
}
#line 3583 "dhcp4_parser.cc"
    break;

  case 697:
#line 2273 "dhcp4_parser.yy"
                           {
    ctx.stack_.back()->set("ncr-protocol", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3592 "dhcp4_parser.cc"
    break;

  case 698:
#line 2279 "dhcp4_parser.yy"
        { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("UDP", ctx.loc2pos(yystack_[0].location))); }
#line 3598 "dhcp4_parser.cc"
    break;

  case 699:
#line 2280 "dhcp4_parser.yy"
        { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("TCP", ctx.loc2pos(yystack_[0].location))); }
#line 3604 "dhcp4_parser.cc"
    break;

  case 700:
#line 2283 "dhcp4_parser.yy"
                       {
    ctx.enter(ctx.NCR_FORMAT);
}
#line 3612 "dhcp4_parser.cc"
    break;

  case 701:
#line 2285 "dhcp4_parser.yy"
             {
    ElementPtr json(new StringElement("JSON", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ncr-format", json);
    ctx.leave();
}
#line 3622 "dhcp4_parser.cc"
    break;

  case 702:
#line 2292 "dhcp4_parser.yy"
                                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3630 "dhcp4_parser.cc"
    break;

  case 703:
#line 2294 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("qualifying-suffix", s);
    ctx.leave();
}
#line 3640 "dhcp4_parser.cc"
    break;

  case 704:
#line 2301 "dhcp4_parser.yy"
                                                         {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("override-no-update", b);
}
#line 3649 "dhcp4_parser.cc"
    break;

  case 705:
#line 2307 "dhcp4_parser.yy"
                                                                 {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("override-client-update", b);
}
#line 3658 "dhcp4_parser.cc"
    break;

  case 706:
#line 2313 "dhcp4_parser.yy"
                                             {
    ctx.enter(ctx.REPLACE_CLIENT_NAME);
}
#line 3666 "dhcp4_parser.cc"
    break;

  case 707:
#line 2315 "dhcp4_parser.yy"
                                       {
    ctx.stack_.back()->set("replace-client-name", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3675 "dhcp4_parser.cc"
    break;

  case 708:
#line 2321 "dhcp4_parser.yy"
                                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3683 "dhcp4_parser.cc"
    break;

  case 709:
#line 2323 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("generated-prefix", s);
    ctx.leave();
}
#line 3693 "dhcp4_parser.cc"
    break;

  case 710:
#line 2330 "dhcp4_parser.yy"
                                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3701 "dhcp4_parser.cc"
    break;

  case 711:
#line 2332 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-set", s);
    ctx.leave();
}
#line 3711 "dhcp4_parser.cc"
    break;

  case 712:
#line 2339 "dhcp4_parser.yy"
                                                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3719 "dhcp4_parser.cc"
    break;

  case 713:
#line 2341 "dhcp4_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-replacement", s);
    ctx.leave();
}
#line 3729 "dhcp4_parser.cc"
    break;

  case 714:
#line 2350 "dhcp4_parser.yy"
                               {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-control", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.CONFIG_CONTROL);
}
#line 3740 "dhcp4_parser.cc"
    break;

  case 715:
#line 2355 "dhcp4_parser.yy"
                                                            {
    // No config control params are required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3750 "dhcp4_parser.cc"
    break;

  case 716:
#line 2361 "dhcp4_parser.yy"
                                   {
    // Parse the config-control map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 3760 "dhcp4_parser.cc"
    break;

  case 717:
#line 2365 "dhcp4_parser.yy"
                                       {
    // No config_control params are required
    // parsing completed
}
#line 3769 "dhcp4_parser.cc"
    break;

  case 722:
#line 2380 "dhcp4_parser.yy"
                                   {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-databases", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.CONFIG_DATABASE);
}
#line 3780 "dhcp4_parser.cc"
    break;

  case 723:
#line 2385 "dhcp4_parser.yy"
                                                      {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3789 "dhcp4_parser.cc"
    break;

  case 724:
#line 2390 "dhcp4_parser.yy"
                                                             {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-fetch-wait-time", value);
}
#line 3798 "dhcp4_parser.cc"
    break;

  case 725:
#line 2397 "dhcp4_parser.yy"
                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("loggers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.LOGGERS);
}
#line 3809 "dhcp4_parser.cc"
    break;

  case 726:
#line 2402 "dhcp4_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3818 "dhcp4_parser.cc"
    break;

  case 729:
#line 2414 "dhcp4_parser.yy"
                             {
    ElementPtr l(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(l);
    ctx.stack_.push_back(l);
}
#line 3828 "dhcp4_parser.cc"
    break;

  case 730:
#line 2418 "dhcp4_parser.yy"
                               {
    ctx.stack_.pop_back();
}
#line 3836 "dhcp4_parser.cc"
    break;

  case 740:
#line 2435 "dhcp4_parser.yy"
                                     {
    ElementPtr dl(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("debuglevel", dl);
}
#line 3845 "dhcp4_parser.cc"
    break;

  case 741:
#line 2440 "dhcp4_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3853 "dhcp4_parser.cc"
    break;

  case 742:
#line 2442 "dhcp4_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("severity", sev);
    ctx.leave();
}
#line 3863 "dhcp4_parser.cc"
    break;

  case 743:
#line 2448 "dhcp4_parser.yy"
                                    {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output_options", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OUTPUT_OPTIONS);
}
#line 3874 "dhcp4_parser.cc"
    break;

  case 744:
#line 2453 "dhcp4_parser.yy"
                                                                    {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3883 "dhcp4_parser.cc"
    break;

  case 747:
#line 2462 "dhcp4_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 3893 "dhcp4_parser.cc"
    break;

  case 748:
#line 2466 "dhcp4_parser.yy"
                                    {
    ctx.stack_.pop_back();
}
#line 3901 "dhcp4_parser.cc"
    break;

  case 756:
#line 2481 "dhcp4_parser.yy"
               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3909 "dhcp4_parser.cc"
    break;

  case 757:
#line 2483 "dhcp4_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output", sev);
    ctx.leave();
}
#line 3919 "dhcp4_parser.cc"
    break;

  case 758:
#line 2489 "dhcp4_parser.yy"
                           {
    ElementPtr flush(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flush", flush);
}
#line 3928 "dhcp4_parser.cc"
    break;

  case 759:
#line 2494 "dhcp4_parser.yy"
                               {
    ElementPtr maxsize(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxsize", maxsize);
}
#line 3937 "dhcp4_parser.cc"
    break;

  case 760:
#line 2499 "dhcp4_parser.yy"
                             {
    ElementPtr maxver(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxver", maxver);
}
#line 3946 "dhcp4_parser.cc"
    break;

  case 761:
#line 2504 "dhcp4_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3954 "dhcp4_parser.cc"
    break;

  case 762:
#line 2506 "dhcp4_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pattern", sev);
    ctx.leave();
}
#line 3964 "dhcp4_parser.cc"
    break;

  case 763:
#line 2514 "dhcp4_parser.yy"
                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3972 "dhcp4_parser.cc"
    break;

  case 764:
#line 2516 "dhcp4_parser.yy"
               {
    ElementPtr allocation_type(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("allocation-type", allocation_type);
    ctx.leave();
}
#line 3982 "dhcp4_parser.cc"
    break;

  case 765:
#line 2522 "dhcp4_parser.yy"
                                 {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.CONFIG_DATABASE);
}
#line 3993 "dhcp4_parser.cc"
    break;

  case 766:
#line 2527 "dhcp4_parser.yy"
                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4002 "dhcp4_parser.cc"
    break;

  case 767:
#line 2532 "dhcp4_parser.yy"
                                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4010 "dhcp4_parser.cc"
    break;

  case 768:
#line 2534 "dhcp4_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("configuration-type", prf);
    ctx.leave();
}
#line 4020 "dhcp4_parser.cc"
    break;

  case 769:
#line 2540 "dhcp4_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4028 "dhcp4_parser.cc"
    break;

  case 770:
#line 2542 "dhcp4_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("instance-id", prf);
    ctx.leave();
}
#line 4038 "dhcp4_parser.cc"
    break;

  case 771:
#line 2548 "dhcp4_parser.yy"
                                                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4046 "dhcp4_parser.cc"
    break;

  case 772:
#line 2550 "dhcp4_parser.yy"
              {
    ctx.stack_.back()->set("lawful-interception-parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 4055 "dhcp4_parser.cc"
    break;

  case 773:
#line 2555 "dhcp4_parser.yy"
                                 {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("master-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.MASTER_DATABASE);
}
#line 4066 "dhcp4_parser.cc"
    break;

  case 774:
#line 2560 "dhcp4_parser.yy"
                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4075 "dhcp4_parser.cc"
    break;

  case 775:
#line 2565 "dhcp4_parser.yy"
                                                               {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-address-utilization", n);
}
#line 4084 "dhcp4_parser.cc"
    break;

  case 776:
#line 2570 "dhcp4_parser.yy"
                                                       {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-statement-tries", n);
}
#line 4093 "dhcp4_parser.cc"
    break;

  case 777:
#line 2575 "dhcp4_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("notifications", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKET);
}
#line 4104 "dhcp4_parser.cc"
    break;

  case 778:
#line 2580 "dhcp4_parser.yy"
                                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4113 "dhcp4_parser.cc"
    break;

  case 779:
#line 2585 "dhcp4_parser.yy"
                                                            {
    ElementPtr l(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-set-description", l);
}
#line 4122 "dhcp4_parser.cc"
    break;

  case 780:
#line 2590 "dhcp4_parser.yy"
                                           {
    ElementPtr l(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-set-id", l);
}
#line 4131 "dhcp4_parser.cc"
    break;

  case 781:
#line 2595 "dhcp4_parser.yy"
                                                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4139 "dhcp4_parser.cc"
    break;

  case 782:
#line 2597 "dhcp4_parser.yy"
              {
    ctx.stack_.back()->set("policy-engine-parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 4148 "dhcp4_parser.cc"
    break;

  case 783:
#line 2602 "dhcp4_parser.yy"
                                                         {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("privacy-history-size", prf);
}
#line 4157 "dhcp4_parser.cc"
    break;

  case 784:
#line 2607 "dhcp4_parser.yy"
                                                             {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("privacy-valid-lifetime", prf);
}
#line 4166 "dhcp4_parser.cc"
    break;

  case 785:
#line 2612 "dhcp4_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4174 "dhcp4_parser.cc"
    break;

  case 786:
#line 2614 "dhcp4_parser.yy"
               {
    ElementPtr protocol(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("protocol", protocol);
    ctx.leave();
}
#line 4184 "dhcp4_parser.cc"
    break;

  case 787:
#line 2620 "dhcp4_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4192 "dhcp4_parser.cc"
    break;

  case 788:
#line 2622 "dhcp4_parser.yy"
               {
    ElementPtr cp(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ssl-cert", cp);
    ctx.leave();
}
#line 4202 "dhcp4_parser.cc"
    break;

  case 789:
#line 2628 "dhcp4_parser.yy"
                                                           {
    ElementPtr offset(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("v4-psid-offset", offset);
}
#line 4211 "dhcp4_parser.cc"
    break;

  case 790:
#line 2633 "dhcp4_parser.yy"
                                                     {
    ElementPtr psid_len(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("v4-psid-len", psid_len);
}
#line 4220 "dhcp4_parser.cc"
    break;

  case 791:
#line 2640 "dhcp4_parser.yy"
                                                   {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("v4-excluded-psids", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4231 "dhcp4_parser.cc"
    break;

  case 792:
#line 2645 "dhcp4_parser.yy"
                                                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4240 "dhcp4_parser.cc"
    break;


#line 4244 "dhcp4_parser.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[+yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yy_error_token_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yy_error_token_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  Dhcp4Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  Dhcp4Parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    std::ptrdiff_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */
    if (!yyla.empty ())
      {
        symbol_number_type yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];

        int yyn = yypact_[+yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yy_error_token_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short Dhcp4Parser::yypact_ninf_ = -985;

  const signed char Dhcp4Parser::yytable_ninf_ = -1;

  const short
  Dhcp4Parser::yypact_[] =
  {
     570,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,    40,    34,    63,    87,    89,   114,
     125,   127,   129,   141,   158,   160,   168,   250,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,    34,  -169,    36,   124,    67,   515,
     183,   240,   -32,   353,   533,   172,   599,   201,  -985,    66,
      71,   175,    60,   101,  -985,    39,  -985,  -985,  -985,  -985,
    -985,   216,   280,   291,  -985,  -985,  -985,  -985,  -985,  -985,
     293,   312,   314,   357,   361,   362,   371,   372,   373,  -985,
     384,   386,   393,   394,   395,  -985,  -985,  -985,   396,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   397,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,   398,   399,   402,   404,   406,  -985,
      41,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   407,  -985,    70,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   408,  -985,  -985,  -985,  -985,  -985,   411,   413,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,    77,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,   416,  -985,  -985,  -985,  -985,  -985,
      85,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   253,   272,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,   295,  -985,  -985,   417,  -985,  -985,  -985,   418,
    -985,  -985,   336,   420,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   421,   422,  -985,
    -985,  -985,  -985,   352,   425,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,    97,
    -985,  -985,  -985,  -985,  -985,   426,  -985,  -985,   427,  -985,
     428,   429,  -985,  -985,   430,   435,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,   100,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   441,   119,  -985,  -985,  -985,  -985,    34,    34,  -985,
     223,   445,    36,  -985,   447,   449,   450,   248,   251,   254,
     455,   458,   460,   462,   465,   470,   269,   273,   274,   278,
     281,   270,   279,   285,   283,   491,   289,   290,   292,   294,
     296,   495,   496,   498,   297,   504,   506,   507,   508,   511,
     514,   516,   517,   519,   521,   522,   523,   322,   526,   527,
     530,   534,   543,   544,   545,   548,   556,   558,   559,   560,
     359,   363,   374,   375,  -985,   124,  -985,   562,   576,   587,
     388,    67,  -985,   591,   594,   595,   596,   597,   598,   401,
     600,   601,   602,   618,   619,   419,   423,   515,  -985,   625,
     424,   183,  -985,   630,   632,   633,   634,   635,   636,   637,
     638,  -985,   240,  -985,   640,   641,   439,   662,   663,   664,
     466,  -985,   353,   667,   467,   476,  -985,   533,   675,   682,
     683,   686,   -71,  -985,   487,   693,   694,   492,   696,   494,
     497,   698,   699,   499,   500,   706,   707,   708,   709,   599,
    -985,   710,   509,   201,  -985,  -985,  -985,   711,   712,  -985,
     713,   715,   720,  -985,  -985,  -985,   512,   524,   525,   721,
     722,   727,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   529,  -985,  -985,  -985,  -985,  -985,   154,   531,   532,
    -985,   728,   731,   735,   736,   748,   550,   130,   752,   753,
     754,   755,   769,  -985,   770,   771,   772,   773,   575,   577,
     777,   778,   578,   580,   780,   781,  -985,  -985,  -985,  -985,
    -985,   785,   265,   299,  -985,  -985,   585,   586,   588,   788,
     589,   590,  -985,   785,   593,   793,   603,   796,  -985,  -985,
    -985,   604,  -985,  -985,   785,   605,   606,   607,   608,   610,
     611,   612,  -985,   613,   614,  -985,   615,   616,   617,  -985,
    -985,   620,  -985,  -985,  -985,   621,   748,    34,    34,  -985,
    -985,   622,   623,  -985,   624,  -985,  -985,    14,   446,  -985,
    -985,   154,   626,   627,   628,  -985,   797,  -985,  -985,    34,
     124,   201,    67,   779,  -985,  -985,  -985,   609,   609,   799,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   800,
     801,   817,   818,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   211,   824,   828,   829,    17,   -42,    65,   -30,   599,
    -985,  -985,   830,   609,  -985,  -985,   609,    74,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   831,
    -985,  -985,  -985,  -985,   210,  -985,    34,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   799,  -985,   121,
     155,   163,  -985,   209,  -985,  -985,  -985,  -985,  -985,  -985,
     835,   836,   837,   838,   839,  -985,  -985,  -985,  -985,   840,
     841,   842,   843,   844,   845,   846,  -985,  -985,  -985,   213,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,   219,  -985,   798,   714,  -985,  -985,
     847,   848,  -985,  -985,   849,   851,  -985,  -985,   850,   854,
    -985,  -985,   852,   856,  -985,  -985,  -985,  -985,  -985,  -985,
      81,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   122,  -985,
    -985,   855,   857,  -985,  -985,   858,   859,  -985,   861,   862,
     863,   864,   865,   866,   242,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,   867,   868,   869,  -985,   245,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   255,  -985,
    -985,  -985,   870,  -985,   871,  -985,  -985,  -985,   257,  -985,
    -985,  -985,  -985,  -985,   264,  -985,   174,  -985,   271,   282,
     872,  -985,  -985,  -985,   288,  -985,  -985,  -985,  -985,   646,
    -985,   873,   860,  -985,  -985,  -985,  -985,   874,   875,   877,
    -985,  -985,  -985,   876,   779,  -985,   880,   881,   882,   883,
     670,   669,   681,   680,   684,   886,   888,   889,   890,   688,
     689,   690,   691,   692,   695,   697,   895,   896,   609,  -985,
    -985,   609,  -985,   799,   515,  -985,   800,   353,  -985,   801,
     533,  -985,   817,   310,  -985,   818,   211,  -985,   144,   824,
    -985,   240,  -985,   828,   172,  -985,   829,   700,   701,   702,
     703,   704,   705,    17,  -985,   716,   717,   719,   -42,  -985,
     899,   901,    65,  -985,   718,   902,   723,   909,   -30,  -985,
    -985,   -28,   830,  -985,  -985,  -985,   724,    74,  -985,  -985,
     908,   912,   183,  -985,   831,   913,  -985,  -985,  -985,   725,
    -985,   358,   726,   729,   730,  -985,  -985,  -985,  -985,  -985,
     732,   733,   738,   739,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,   742,   744,  -985,   298,  -985,   301,  -985,   910,  -985,
     911,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   323,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
     916,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
     914,   918,  -985,  -985,  -985,  -985,  -985,   915,  -985,   338,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,   747,   761,  -985,  -985,   763,  -985,    34,  -985,
    -985,   924,  -985,  -985,  -985,  -985,  -985,   339,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,   766,   340,  -985,   785,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   310,
    -985,   925,   764,  -985,   144,  -985,  -985,  -985,  -985,  -985,
    -985,   930,   767,   938,   -28,  -985,  -985,  -985,  -985,  -985,
     775,  -985,  -985,   956,  -985,   782,  -985,  -985,   969,  -985,
    -985,   243,  -985,    37,   969,  -985,  -985,   974,   975,   982,
    -985,   356,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   988,
     789,   792,   794,  1002,    37,  -985,   809,  -985,  -985,  -985,
     810,  -985,  -985,  -985
  };

  const short
  Dhcp4Parser::yydefact_[] =
  {
       0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
      20,    22,    24,    26,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     1,    43,
      36,    32,    31,    28,    29,    30,    35,     3,    33,    34,
      56,     5,    63,     7,   174,     9,   337,    11,   524,    13,
     551,    15,   449,    17,   457,    19,   494,    21,   300,    23,
     666,    25,   716,    27,    45,    39,     0,     0,     0,     0,
       0,   553,     0,   459,   496,     0,     0,     0,    47,     0,
      46,     0,     0,    40,    61,     0,    58,    60,   714,   163,
     189,     0,     0,     0,   572,   574,   576,   187,   196,   198,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   137,
       0,     0,     0,     0,     0,   145,   152,   154,     0,   329,
     447,   486,   404,   539,   541,   397,   265,   600,   543,   292,
     313,     0,   279,   626,   648,   664,   156,   158,   725,   765,
     767,   769,   773,   777,     0,     0,     0,     0,     0,   127,
       0,    65,    67,    68,    69,    70,    71,   102,   103,   104,
      72,   100,    89,    90,    91,   108,   109,   110,   111,   112,
     113,   106,   107,   114,   115,   116,    75,    76,    97,    77,
      78,    79,   117,    83,    84,    73,   101,    74,    81,    82,
      95,    96,    98,    92,    93,    94,    80,    85,    86,    87,
      88,    99,   105,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   176,   178,   182,     0,   173,     0,   165,   167,
     168,   169,   170,   171,   172,   385,   387,   389,   516,   383,
     391,     0,   395,   393,   596,   763,   791,     0,     0,   382,
     341,   342,   343,   344,   345,   366,   367,   368,   356,   357,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     339,   348,   361,   362,   363,   349,   351,   352,   354,   350,
     346,   347,   364,   365,   353,   358,   359,   360,   355,   378,
     381,   380,   379,   537,     0,   536,   530,   531,   534,   529,
       0,   526,   528,   532,   533,   535,   594,   582,   584,   588,
     586,   592,   590,   578,   571,   565,   569,   570,     0,   554,
     555,   566,   567,   568,   562,   557,   563,   559,   560,   561,
     564,   558,     0,   476,   245,     0,   480,   478,   483,     0,
     472,   473,     0,   460,   461,   463,   475,   464,   465,   466,
     482,   467,   468,   469,   470,   471,   510,     0,     0,   508,
     509,   512,   513,     0,   497,   498,   500,   501,   502,   503,
     504,   505,   506,   507,   309,   311,   771,   781,   304,     0,
     302,   305,   306,   307,   308,     0,   702,   689,     0,   692,
       0,     0,   696,   700,     0,     0,   706,   708,   710,   712,
     687,   685,   686,     0,   668,   670,   671,   672,   673,   674,
     675,   676,   677,   682,   678,   679,   680,   681,   683,   684,
     722,     0,     0,   718,   720,   721,    44,     0,     0,    37,
       0,     0,     0,    57,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    55,     0,    64,     0,     0,     0,
       0,     0,   175,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   338,     0,
       0,     0,   525,     0,     0,     0,     0,     0,     0,     0,
       0,   552,     0,   450,     0,     0,     0,     0,     0,     0,
       0,   458,     0,     0,     0,     0,   495,     0,     0,     0,
       0,     0,     0,   301,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     667,     0,     0,     0,   717,    48,    41,     0,     0,    59,
       0,     0,     0,   139,   140,   141,     0,     0,     0,     0,
       0,     0,   128,   129,   130,   131,   132,   133,   134,   135,
     136,     0,   161,   162,   142,   143,   144,     0,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   625,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   779,   780,   783,   784,
      66,     0,     0,     0,   186,   166,     0,     0,     0,     0,
       0,     0,   403,     0,     0,     0,     0,     0,   790,   789,
     340,     0,   775,   527,     0,     0,     0,     0,     0,     0,
       0,     0,   556,     0,     0,   474,     0,     0,     0,   485,
     462,     0,   514,   515,   499,     0,     0,     0,     0,   303,
     688,     0,     0,   691,     0,   694,   695,     0,     0,   704,
     705,     0,     0,     0,     0,   669,     0,   724,   719,     0,
       0,     0,     0,     0,   573,   575,   577,     0,     0,   200,
     138,   147,   148,   149,   150,   151,   146,   153,   155,   331,
     451,   488,   406,    38,   540,   542,   399,   400,   401,   402,
     398,     0,     0,   545,   294,     0,     0,     0,     0,     0,
     157,   159,     0,     0,   768,   770,     0,     0,    49,   177,
     180,   181,   179,   184,   185,   183,   386,   388,   390,   518,
     384,   392,   396,   394,     0,   764,    45,   538,   595,   583,
     585,   589,   587,   593,   591,   579,   477,   246,   481,   479,
     484,   511,   310,   312,   772,   782,   703,   690,   693,   698,
     699,   697,   701,   707,   709,   711,   713,   200,    42,     0,
       0,     0,   194,     0,   191,   193,   232,   238,   240,   242,
       0,     0,     0,     0,     0,   254,   256,   258,   260,     0,
       0,     0,     0,     0,     0,     0,   785,   787,   231,     0,
     206,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   221,   222,   223,   218,   224,   225,   226,   219,   220,
     227,   228,   229,   230,     0,   204,     0,   201,   202,   335,
       0,   332,   333,   455,     0,   452,   453,   492,     0,   489,
     490,   410,     0,   407,   408,   274,   275,   276,   277,   278,
       0,   267,   269,   270,   271,   272,   273,   604,     0,   602,
     549,     0,   546,   547,   298,     0,   295,   296,     0,     0,
       0,     0,     0,     0,     0,   315,   317,   318,   319,   320,
     321,   322,     0,     0,     0,   288,     0,   281,   283,   284,
     285,   286,   287,   643,   645,   634,   632,   633,     0,   628,
     630,   631,     0,   659,     0,   662,   655,   656,     0,   650,
     652,   653,   654,   657,     0,   729,     0,   727,     0,     0,
       0,   642,   639,   640,     0,   635,   637,   638,   641,    51,
     522,     0,   519,   520,   580,   598,   599,     0,     0,     0,
      62,   715,   164,     0,     0,   190,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   188,
     197,     0,   199,     0,     0,   330,     0,   459,   448,     0,
     496,   487,     0,     0,   405,     0,     0,   266,   606,     0,
     601,   553,   544,     0,     0,   293,     0,     0,     0,     0,
       0,     0,     0,     0,   314,     0,     0,     0,     0,   280,
       0,     0,     0,   627,     0,     0,     0,     0,     0,   649,
     665,     0,     0,   726,   766,   774,     0,     0,   778,    53,
       0,    52,     0,   517,     0,     0,   597,   792,   723,     0,
     192,     0,     0,     0,     0,   244,   247,   248,   249,   250,
       0,     0,     0,     0,   262,   263,   251,   252,   253,   264,
     776,     0,     0,   207,     0,   203,     0,   334,     0,   454,
       0,   491,   446,   429,   430,   431,   417,   418,   434,   435,
     436,   420,   421,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   414,   415,   416,   427,   428,   426,     0,   412,
     419,   432,   433,   422,   423,   424,   425,   409,   268,   622,
       0,   620,   621,   613,   614,   618,   619,   615,   616,   617,
       0,   607,   608,   610,   611,   612,   603,     0,   548,     0,
     297,   323,   324,   325,   326,   327,   328,   316,   289,   290,
     291,   282,     0,     0,   629,   658,     0,   661,     0,   651,
     743,     0,   741,   739,   733,   737,   738,     0,   731,   735,
     736,   734,   728,   647,   636,    50,     0,     0,   521,     0,
     195,   234,   235,   236,   237,   233,   239,   241,   243,   255,
     257,   259,   261,   786,   788,   205,   336,   456,   493,     0,
     411,     0,     0,   605,     0,   550,   299,   644,   646,   660,
     663,     0,     0,     0,     0,   730,    54,   523,   581,   413,
       0,   624,   609,     0,   740,     0,   732,   623,     0,   742,
     747,     0,   745,     0,     0,   744,   756,     0,     0,     0,
     761,     0,   749,   751,   752,   753,   754,   755,   746,     0,
       0,     0,     0,     0,     0,   748,     0,   758,   759,   760,
       0,   750,   757,   762
  };

  const short
  Dhcp4Parser::yypgoto_[] =
  {
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,    -4,  -985,  -569,  -985,   341,
    -985,  -985,  -985,  -985,   150,  -985,  -616,  -985,  -985,  -985,
     -67,  -985,  -985,  -985,   648,  -985,  -985,  -985,  -985,   376,
     537,   -24,     7,    26,    28,    29,    30,    32,    33,  -985,
    -985,  -985,  -985,    43,    44,    46,    47,    48,    49,  -985,
     349,    50,  -985,    51,  -985,    54,  -985,    61,  -985,    62,
    -985,  -985,  -985,  -985,   369,   581,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,    99,  -985,  -985,  -985,  -985,  -985,  -985,   284,
    -985,    75,  -985,  -684,    82,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,   -48,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,    68,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,    45,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,    53,  -985,  -985,  -985,    58,
     546,  -985,  -985,  -985,  -985,  -985,  -985,  -985,    52,  -985,
    -985,  -985,  -985,  -985,  -985,  -984,  -985,  -985,  -985,    80,
    -985,  -985,  -985,    83,   582,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -983,  -985,   -53,  -985,    56,  -985,    64,
    -985,  -985,   -50,  -985,  -985,  -985,  -985,    76,  -985,  -985,
    -129,   -62,  -985,  -985,  -985,  -985,  -985,    84,  -985,  -985,
    -985,    88,  -985,   564,  -985,   -69,  -985,  -985,  -985,  -985,
    -985,   -46,  -985,  -985,  -985,  -985,  -985,   -36,  -985,  -985,
    -985,    86,  -985,  -985,  -985,    90,  -985,   555,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,    35,
    -985,  -985,  -985,    42,   583,  -985,  -985,   -55,  -985,   -13,
    -985,   -57,  -985,  -985,  -985,    78,  -985,  -985,  -985,    91,
    -985,   584,     0,  -985,     6,  -985,    10,  -985,   333,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -982,  -985,  -985,  -985,
    -985,  -985,    92,  -985,  -985,  -985,  -122,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,    72,  -985,    59,  -725,  -985,
    -724,  -985,  -985,  -985,  -985,  -985,    55,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,   366,   549,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,   409,   552,
    -985,  -985,  -985,  -985,  -985,  -985,    57,  -985,  -985,  -127,
    -985,  -985,  -985,  -985,  -985,  -985,  -141,  -985,  -985,  -157,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,  -985,
    -985,  -985,  -985,  -985,  -985,  -985,  -985
  };

  const short
  Dhcp4Parser::yydefgoto_[] =
  {
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    78,    37,    38,    65,   724,
      82,    83,    39,    64,    79,    80,   749,   959,  1060,  1061,
     828,    41,    66,    85,    86,    87,   421,    43,    67,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   445,   162,   163,   164,   165,   166,   167,   168,   451,
     716,   169,   452,   170,   453,   171,   472,   172,   473,   173,
     174,   175,   176,   425,   217,   218,    45,    68,   219,   487,
     220,   488,   752,   221,   489,   755,   222,   177,   433,   178,
     426,   803,   804,   805,   973,   179,   434,   180,   435,   856,
     857,   858,  1001,   829,   830,   831,   976,  1205,   832,   977,
     833,   978,   834,   979,   835,   836,   525,   837,   838,   839,
     840,   841,   842,   843,   844,   985,   845,   986,   846,   987,
     847,   988,   848,   849,   850,   181,   462,   880,   881,   882,
     883,   884,   885,   886,   182,   468,   916,   917,   918,   919,
     920,   183,   465,   895,   896,   897,  1024,    59,    75,   369,
     370,   371,   538,   372,   539,   184,   466,   904,   905,   906,
     907,   908,   909,   910,   911,   185,   455,   860,   861,   862,
    1004,    47,    69,   259,   260,   261,   497,   262,   493,   263,
     494,   264,   495,   265,   498,   266,   501,   267,   500,   186,
     461,   730,   269,   187,   458,   872,   873,   874,  1013,  1128,
    1129,   188,   456,    53,    72,   864,   865,   866,  1007,    55,
      73,   332,   333,   334,   335,   336,   337,   338,   524,   339,
     528,   340,   527,   341,   342,   529,   343,   189,   457,   868,
     869,   870,  1010,    57,    74,   353,   354,   355,   356,   357,
     533,   358,   359,   360,   361,   271,   496,   961,   962,   963,
    1062,    49,    70,   290,   291,   292,   509,   190,   459,   191,
     460,   192,   464,   891,   892,   893,  1021,    51,    71,   308,
     309,   310,   193,   430,   194,   431,   195,   432,   314,   520,
     966,  1065,   315,   514,   316,   515,   317,   517,   318,   516,
     319,   519,   320,   518,   321,   513,   278,   502,   967,   196,
     463,   888,   889,  1018,  1150,  1151,  1152,  1153,  1154,  1221,
    1155,   197,   198,   469,   928,   929,   954,   955,   930,  1040,
     931,  1041,   958,   199,   470,   938,   939,   940,   941,  1045,
     942,   943,  1047,   200,   471,    61,    76,   393,   394,   395,
     396,   546,   397,   398,   548,   399,   400,   401,   551,   791,
     402,   552,   403,   545,   404,   405,   406,   555,   407,   556,
     408,   557,   409,   558,   201,   424,    63,    77,   412,   413,
     414,   561,   415,   202,   474,   946,   947,  1051,  1187,  1188,
    1189,  1190,  1233,  1191,  1231,  1251,  1252,  1253,  1261,  1262,
    1263,  1269,  1264,  1265,  1266,  1267,  1273,   279,   503,   203,
     475,   204,   476,   205,   477,   373,   540,   206,   478,   295,
     851,   207,   479,   208,   209,   374,   541,   210,   211,   852,
     996,   853,   997,   280,   281,   282,   504
  };

  const short
  Dhcp4Parser::yytable_[] =
  {
     149,   216,   239,   285,   304,   351,   330,   349,   368,   390,
     322,    36,   274,   223,   272,   293,   306,   286,   344,   362,
     288,   391,   956,   957,   854,   331,   350,   762,   352,  1123,
    1124,  1136,   789,   270,   289,   305,   723,    81,   768,    29,
      28,    30,   422,    31,   485,   240,    84,   423,   120,   486,
     364,   365,   123,   124,   324,   224,   273,   294,   307,   948,
     345,   363,   949,   392,   123,   124,   123,   124,   419,   275,
      40,   311,   416,   491,   417,   276,   241,   312,   492,   277,
     507,   313,   212,   213,  1016,   508,   214,  1017,   511,   215,
     912,   913,   914,   512,    42,   242,    44,   243,   244,   245,
     542,   246,   247,   559,   420,   543,   366,   723,   560,   932,
     933,   934,   248,   249,   367,   250,   251,   252,   253,   254,
     255,    46,   563,   256,   485,  1019,   287,   564,  1020,   970,
     257,   258,    48,   268,    50,    88,    52,  1180,    89,  1181,
    1182,   898,   899,   900,   901,   902,   903,    90,    54,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   563,   123,
     124,   123,   124,   971,   148,    56,   491,    58,   123,   124,
     790,   972,    94,    95,    96,    60,   935,  1052,   148,   418,
    1053,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   923,   924,  1256,   120,   121,  1257,  1258,  1259,  1260,
     923,   924,   974,   410,   411,   122,   998,   975,   123,   124,
     427,   999,   998,   125,   120,   121,   324,  1000,   126,   127,
     726,   727,   728,   729,   128,  1123,  1124,  1136,   123,   124,
      32,    33,    34,    35,   129,  1033,  1254,   130,  1038,  1255,
    1034,  1139,  1140,  1039,   131,   132,   950,    62,  1042,   133,
    1048,   521,   134,  1043,   121,  1049,   135,   559,    94,    95,
      96,   148,  1050,   148,   998,   522,   283,   123,   124,  1054,
     148,   231,   750,   751,   428,   998,   136,   137,   138,   232,
    1055,  1057,   233,   364,   365,   429,  1058,   436,   139,   140,
     141,   998,   142,   523,   507,   143,  1215,   144,   145,  1216,
     146,   147,   711,   712,   713,   714,   437,  1094,   438,   753,
     754,   121,   875,   876,   877,   878,  1219,   879,   303,   964,
     148,  1220,   956,   957,   123,   124,    92,    93,    94,    95,
      96,   542,  1234,   511,   531,   296,  1226,  1235,  1237,   366,
     148,   297,   298,   299,   300,   301,   302,   367,   303,  1274,
     536,   439,   284,   715,  1275,   440,   441,   100,   101,   102,
     103,   104,   105,   106,   107,   442,   443,   444,   148,   112,
     113,   114,   115,   116,   117,   118,   119,   323,   446,   148,
     447,   121,   324,  1201,  1202,  1203,  1204,   448,   449,   450,
     454,   467,   480,   481,   123,   124,   482,   230,   483,   125,
     484,   490,   499,   565,   566,   505,   232,   506,   149,   233,
     510,   526,   530,   532,   216,   534,   535,   234,   537,   567,
     544,   547,   549,   550,   553,   324,   223,   325,   326,   554,
     239,   327,   328,   329,   285,   562,   148,   123,   124,   568,
     274,   570,   272,   571,   572,   304,   293,   573,   286,   576,
     574,   288,   577,   575,   578,   330,   579,   306,   351,   580,
     349,   270,   136,   137,   581,   289,   582,   344,   224,   587,
     583,   584,   362,   240,   331,   585,   305,   588,   586,   350,
     590,   352,   390,   589,   273,   591,   592,   593,   294,   597,
     598,   594,   599,   595,   391,   596,   600,   275,   601,   307,
     602,   603,   604,   276,   241,   605,   148,   277,   606,   345,
     607,   608,   311,   609,   363,   610,   611,   612,   312,   613,
     614,   615,   313,   242,   616,   243,   244,   245,   617,   246,
     247,    92,    93,    94,    95,    96,   392,   618,   619,   620,
     248,   249,   621,   250,   251,   252,   253,   254,   255,   148,
     622,   256,   623,   624,   625,   626,   631,   287,   257,   258,
     627,   268,   100,   101,   102,   103,   104,   105,   106,   107,
     632,   628,   629,  1238,   112,   113,   114,   115,   116,   117,
     118,   633,   225,   226,   227,   636,   121,   634,   637,   638,
     639,   640,   641,   792,   643,   644,   645,   228,   642,   123,
     124,   229,   230,   231,   125,   324,   346,   325,   326,   347,
     348,   232,   646,   647,   233,   128,   648,   123,   124,   651,
     649,   652,   234,   149,   654,   216,   655,   656,   657,   658,
     659,   660,   661,   806,   663,   664,   665,   223,   807,   808,
     809,   810,   811,   812,   813,   814,   815,   816,   817,   818,
     819,   820,   821,   822,   823,   824,   666,   667,   668,   915,
     925,   671,   390,   784,   785,   669,   672,   136,   137,   675,
     951,   921,   926,   936,   391,   673,   676,   677,   235,   224,
     678,   324,   952,   123,   124,   798,   680,   681,   682,   683,
     684,   685,   687,   688,   686,   236,   237,   238,   689,   690,
     691,   692,   693,   694,   696,   699,   697,  1003,   704,   700,
     701,   148,   702,   922,   927,   937,   392,   703,   707,   708,
     705,   706,   709,   719,   953,   710,   720,   717,   718,   148,
     721,   722,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,    30,   725,   731,   732,   733,
     734,   388,   389,     1,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,   735,   736,   737,   738,
     739,   740,   742,   741,   744,   743,   745,   746,   747,   825,
     748,   756,   757,   759,   758,   760,   761,   826,   827,   763,
     764,   766,   797,   802,  1002,   148,   855,   859,   863,   765,
     767,   769,   770,   771,   772,   148,   773,   774,   775,   776,
     777,   778,   779,   780,   867,   871,   781,   782,   786,   787,
     788,   887,   794,   795,   796,   890,   894,   945,   960,   980,
     981,   982,   983,   984,   989,   990,   991,   992,   993,   994,
     995,  1006,  1059,  1005,  1009,  1008,  1011,  1012,  1014,  1015,
    1023,  1022,  1026,  1064,  1025,  1027,  1028,  1029,  1030,  1031,
    1032,  1035,  1036,  1037,  1044,  1046,  1056,  1075,  1076,  1063,
    1069,  1067,  1066,  1068,  1071,  1072,  1073,  1074,  1077,  1078,
    1080,  1079,  1081,  1082,  1083,  1084,  1085,  1086,  1087,  1091,
    1092,  1088,  1089,  1172,  1090,  1173,  1176,  1161,  1162,  1163,
    1164,  1165,  1166,  1178,  1195,  1196,   968,  1199,  1217,  1218,
    1222,  1224,  1223,  1225,  1169,  1168,  1170,  1175,  1232,  1240,
    1177,  1200,  1206,  1193,  1243,  1207,  1208,   239,  1209,  1210,
     330,   351,  1245,   349,  1211,  1212,  1102,   274,  1213,   272,
    1214,  1141,   344,  1227,   304,   362,  1143,   368,  1131,   331,
    1125,  1248,   350,  1145,   352,  1122,   306,  1228,   270,  1229,
    1142,   915,  1236,  1241,  1244,   925,  1250,  1130,  1270,  1271,
     240,  1247,  1144,   921,  1183,   305,  1272,   926,  1249,  1103,
     951,   273,  1276,   936,   345,   285,  1185,   363,  1277,  1278,
    1132,  1279,   952,  1184,   275,  1146,  1280,   293,   307,   286,
     276,   241,   288,  1133,   277,  1282,  1283,   783,  1147,  1134,
    1104,   311,   630,  1135,  1148,   922,   289,   312,  1149,   927,
     242,   313,   243,   244,   245,   937,   246,   247,  1186,  1105,
     793,  1106,  1107,  1108,   953,  1109,  1110,   248,   249,   294,
     250,   251,   252,   253,   254,   255,  1111,  1112,   256,  1113,
    1114,  1115,  1116,  1117,  1118,   257,   258,  1119,   268,  1126,
     569,   801,   635,  1070,  1120,  1121,   799,  1127,  1095,  1160,
    1093,   969,  1159,  1171,  1138,  1167,  1097,  1096,   679,   650,
    1239,  1137,   674,  1099,   653,  1098,   670,   965,  1101,  1198,
    1100,  1158,  1242,  1179,  1197,   944,   662,  1246,   695,  1192,
     800,  1156,  1157,  1268,  1174,   698,  1194,  1281,   287,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1102,     0,     0,     0,     0,  1141,     0,     0,
       0,     0,  1143,     0,  1131,     0,  1125,  1183,     0,  1145,
       0,  1122,     0,     0,  1230,     0,  1142,     0,     0,  1185,
       0,     0,     0,  1130,     0,     0,  1184,     0,  1144,     0,
       0,     0,     0,     0,     0,  1103,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1132,     0,     0,     0,
       0,  1146,     0,     0,     0,     0,     0,     0,     0,  1133,
       0,  1186,     0,     0,  1147,  1134,  1104,     0,     0,  1135,
    1148,     0,     0,     0,  1149,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1105,     0,  1106,  1107,  1108,
       0,  1109,  1110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1111,  1112,     0,  1113,  1114,  1115,  1116,  1117,
    1118,     0,     0,  1119,     0,  1126,     0,     0,     0,     0,
    1120,  1121,     0,  1127
  };

  const short
  Dhcp4Parser::yycheck_[] =
  {
      67,    68,    69,    70,    71,    74,    73,    74,    75,    76,
      72,    15,    69,    68,    69,    70,    71,    70,    73,    74,
      70,    76,   747,   747,   708,    73,    74,   643,    74,  1013,
    1013,  1013,    18,    69,    70,    71,   605,   206,   654,     5,
       0,     7,     3,     9,     3,    69,    10,     8,    80,     8,
     121,   122,    94,    95,    82,    68,    69,    70,    71,   743,
      73,    74,   746,    76,    94,    95,    94,    95,     8,    69,
       7,    71,     6,     3,     3,    69,    69,    71,     8,    69,
       3,    71,    15,    16,     3,     8,    19,     6,     3,    22,
     132,   133,   134,     8,     7,    69,     7,    69,    69,    69,
       3,    69,    69,     3,     3,     8,   177,   676,     8,   139,
     140,   141,    69,    69,   185,    69,    69,    69,    69,    69,
      69,     7,     3,    69,     3,     3,    70,     8,     6,     8,
      69,    69,     7,    69,     7,    11,     7,   165,    14,   167,
     168,   124,   125,   126,   127,   128,   129,    23,     7,    25,
      26,    27,    28,    29,    30,    31,    32,    33,     3,    94,
      95,    94,    95,     8,   206,     7,     3,     7,    94,    95,
     156,     8,    28,    29,    30,     7,   206,     3,   206,     4,
       6,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,   136,   137,   166,    80,    81,   169,   170,   171,   172,
     136,   137,     3,    12,    13,    91,     3,     8,    94,    95,
       4,     8,     3,    99,    80,    81,    82,     8,   104,   105,
     100,   101,   102,   103,   110,  1219,  1219,  1219,    94,    95,
     206,   207,   208,   209,   120,     3,     3,   123,     3,     6,
       8,   107,   108,     8,   130,   131,   182,     7,     3,   135,
       3,     8,   138,     8,    81,     8,   142,     3,    28,    29,
      30,   206,     8,   206,     3,     3,    93,    94,    95,     8,
     206,    98,    17,    18,     4,     3,   162,   163,   164,   106,
       8,     3,   109,   121,   122,     4,     8,     4,   174,   175,
     176,     3,   178,     8,     3,   181,     8,   183,   184,     8,
     186,   187,   158,   159,   160,   161,     4,  1001,     4,    20,
      21,    81,   111,   112,   113,   114,     3,   116,   118,   119,
     206,     8,  1057,  1057,    94,    95,    26,    27,    28,    29,
      30,     3,     3,     3,     8,   105,     8,     8,     8,   177,
     206,   111,   112,   113,   114,   115,   116,   185,   118,     3,
       8,     4,   179,   209,     8,     4,     4,    57,    58,    59,
      60,    61,    62,    63,    64,     4,     4,     4,   206,    69,
      70,    71,    72,    73,    74,    75,    76,    34,     4,   206,
       4,    81,    82,    35,    36,    37,    38,     4,     4,     4,
       4,     4,     4,     4,    94,    95,     4,    97,     4,    99,
       4,     4,     4,   417,   418,     4,   106,     4,   485,   109,
       4,     4,     4,     3,   491,     4,     4,   117,     3,   206,
       4,     4,     4,     4,     4,    82,   491,    84,    85,     4,
     507,    88,    89,    90,   511,     4,   206,    94,    95,     4,
     507,     4,   507,     4,     4,   522,   511,   209,   511,     4,
     209,   511,     4,   209,     4,   532,     4,   522,   537,     4,
     537,   507,   162,   163,     4,   511,   207,   532,   491,   209,
     207,   207,   537,   507,   532,   207,   522,   208,   207,   537,
     207,   537,   559,   208,   507,     4,   207,   207,   511,     4,
       4,   209,     4,   209,   559,   209,   209,   507,     4,   522,
       4,     4,     4,   507,   507,     4,   206,   507,     4,   532,
       4,     4,   522,     4,   537,     4,     4,     4,   522,   207,
       4,     4,   522,   507,     4,   507,   507,   507,     4,   507,
     507,    26,    27,    28,    29,    30,   559,     4,     4,     4,
     507,   507,     4,   507,   507,   507,   507,   507,   507,   206,
       4,   507,     4,     4,     4,   206,     4,   511,   507,   507,
     207,   507,    57,    58,    59,    60,    61,    62,    63,    64,
       4,   207,   207,  1199,    69,    70,    71,    72,    73,    74,
      75,     4,    77,    78,    79,     4,    81,   209,     4,     4,
       4,     4,     4,   157,     4,     4,     4,    92,   207,    94,
      95,    96,    97,    98,    99,    82,    83,    84,    85,    86,
      87,   106,     4,     4,   109,   110,   207,    94,    95,     4,
     207,   207,   117,   700,     4,   702,     4,     4,     4,     4,
       4,     4,     4,    34,     4,     4,   207,   702,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,     4,     4,     4,   736,
     737,     4,   739,   677,   678,   209,   209,   162,   163,     4,
     747,   736,   737,   738,   739,   209,     4,     4,   173,   702,
       4,    82,   747,    94,    95,   699,   209,     4,     4,   207,
       4,   207,     4,     4,   207,   190,   191,   192,   209,   209,
       4,     4,     4,     4,     4,     4,   207,     3,   206,     7,
       7,   206,     7,   736,   737,   738,   739,     7,     7,     7,
     206,   206,     5,     5,   747,   206,     5,   206,   206,   206,
       5,     5,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,     7,   206,     5,     5,     5,
       5,   162,   163,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,     7,     7,     7,     7,
       7,   206,     5,   206,   206,     7,   206,     7,     7,   180,
       5,   206,   206,     5,   206,   206,   206,   188,   189,   206,
       7,     5,     5,    24,     6,   206,     7,     7,     7,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,     7,     7,   206,   206,   206,   206,
     206,     7,   206,   206,   206,     7,     7,     7,     7,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     3,   206,     6,     3,     6,     6,     3,     6,     3,
       3,     6,     3,     3,     6,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,   207,   209,     6,
       4,     6,     8,     6,     4,     4,     4,     4,   207,   209,
       4,   207,     4,     4,     4,   207,   207,   207,   207,     4,
       4,   209,   207,     4,   207,     4,     4,   207,   207,   207,
     207,   207,   207,     4,     6,     3,   766,     4,     8,     8,
       4,     3,     8,     8,   207,   209,   207,   209,     4,     4,
     207,   206,   206,   209,     4,   206,   206,  1004,   206,   206,
    1007,  1010,     4,  1010,   206,   206,  1013,  1004,   206,  1004,
     206,  1018,  1007,   206,  1021,  1010,  1018,  1024,  1013,  1007,
    1013,     5,  1010,  1018,  1010,  1013,  1021,   206,  1004,   206,
    1018,  1038,   206,   209,   207,  1042,     7,  1013,     4,     4,
    1004,   206,  1018,  1038,  1051,  1021,     4,  1042,   206,  1013,
    1057,  1004,     4,  1048,  1007,  1062,  1051,  1010,   209,   207,
    1013,   207,  1057,  1051,  1004,  1018,     4,  1062,  1021,  1062,
    1004,  1004,  1062,  1013,  1004,   206,   206,   676,  1018,  1013,
    1013,  1021,   485,  1013,  1018,  1038,  1062,  1021,  1018,  1042,
    1004,  1021,  1004,  1004,  1004,  1048,  1004,  1004,  1051,  1013,
     691,  1013,  1013,  1013,  1057,  1013,  1013,  1004,  1004,  1062,
    1004,  1004,  1004,  1004,  1004,  1004,  1013,  1013,  1004,  1013,
    1013,  1013,  1013,  1013,  1013,  1004,  1004,  1013,  1004,  1013,
     422,   702,   491,   974,  1013,  1013,   700,  1013,  1003,  1026,
     998,   797,  1024,  1038,  1016,  1033,  1006,  1004,   542,   507,
    1219,  1015,   537,  1009,   511,  1007,   532,   764,  1012,  1064,
    1010,  1023,  1224,  1048,  1062,   739,   522,  1234,   559,  1052,
     701,  1019,  1021,  1254,  1042,   563,  1057,  1274,  1062,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1219,    -1,    -1,    -1,    -1,  1224,    -1,    -1,
      -1,    -1,  1224,    -1,  1219,    -1,  1219,  1234,    -1,  1224,
      -1,  1219,    -1,    -1,  1178,    -1,  1224,    -1,    -1,  1234,
      -1,    -1,    -1,  1219,    -1,    -1,  1234,    -1,  1224,    -1,
      -1,    -1,    -1,    -1,    -1,  1219,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1219,    -1,    -1,    -1,
      -1,  1224,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1219,
      -1,  1234,    -1,    -1,  1224,  1219,  1219,    -1,    -1,  1219,
    1224,    -1,    -1,    -1,  1224,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1219,    -1,  1219,  1219,  1219,
      -1,  1219,  1219,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1219,  1219,    -1,  1219,  1219,  1219,  1219,  1219,
    1219,    -1,    -1,  1219,    -1,  1219,    -1,    -1,    -1,    -1,
    1219,  1219,    -1,  1219
  };

  const short
  Dhcp4Parser::yystos_[] =
  {
       0,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   211,   212,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,     0,     5,
       7,     9,   206,   207,   208,   209,   225,   226,   227,   232,
       7,   241,     7,   247,     7,   286,     7,   391,     7,   471,
       7,   487,     7,   423,     7,   429,     7,   453,     7,   367,
       7,   555,     7,   586,   233,   228,   242,   248,   287,   392,
     472,   488,   424,   430,   454,   368,   556,   587,   225,   234,
     235,   206,   230,   231,    10,   243,   244,   245,    11,    14,
      23,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      80,    81,    91,    94,    95,    99,   104,   105,   110,   120,
     123,   130,   131,   135,   138,   142,   162,   163,   164,   174,
     175,   176,   178,   181,   183,   184,   186,   187,   206,   240,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   262,   263,   264,   265,   266,   267,   268,   271,
     273,   275,   277,   279,   280,   281,   282,   297,   299,   305,
     307,   345,   354,   361,   375,   385,   409,   413,   421,   447,
     477,   479,   481,   492,   494,   496,   519,   531,   532,   543,
     553,   584,   593,   619,   621,   623,   627,   631,   633,   634,
     637,   638,    15,    16,    19,    22,   240,   284,   285,   288,
     290,   293,   296,   477,   479,    77,    78,    79,    92,    96,
      97,    98,   106,   109,   117,   173,   190,   191,   192,   240,
     251,   252,   253,   254,   255,   256,   257,   258,   263,   264,
     265,   266,   267,   268,   271,   273,   275,   277,   279,   393,
     394,   395,   397,   399,   401,   403,   405,   407,   409,   412,
     447,   465,   477,   479,   481,   492,   494,   496,   516,   617,
     643,   644,   645,    93,   179,   240,   405,   407,   412,   447,
     473,   474,   475,   477,   479,   629,   105,   111,   112,   113,
     114,   115,   116,   118,   240,   447,   477,   479,   489,   490,
     491,   492,   494,   496,   498,   502,   504,   506,   508,   510,
     512,   514,   421,    34,    82,    84,    85,    88,    89,    90,
     240,   325,   431,   432,   433,   434,   435,   436,   437,   439,
     441,   443,   444,   446,   477,   479,    83,    86,    87,   240,
     325,   435,   441,   455,   456,   457,   458,   459,   461,   462,
     463,   464,   477,   479,   121,   122,   177,   185,   240,   369,
     370,   371,   373,   625,   635,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   162,   163,
     240,   477,   479,   557,   558,   559,   560,   562,   563,   565,
     566,   567,   570,   572,   574,   575,   576,   578,   580,   582,
      12,    13,   588,   589,   590,   592,     6,     3,     4,     8,
       3,   246,     3,     8,   585,   283,   300,     4,     4,     4,
     493,   495,   497,   298,   306,   308,     4,     4,     4,     4,
       4,     4,     4,     4,     4,   261,     4,     4,     4,     4,
       4,   269,   272,   274,     4,   386,   422,   448,   414,   478,
     480,   410,   346,   520,   482,   362,   376,     4,   355,   533,
     544,   554,   276,   278,   594,   620,   622,   624,   628,   632,
       4,     4,     4,     4,     4,     3,     8,   289,   291,   294,
       4,     3,     8,   398,   400,   402,   466,   396,   404,     4,
     408,   406,   517,   618,   646,     4,     4,     3,     8,   476,
       4,     3,     8,   515,   503,   505,   509,   507,   513,   511,
     499,     8,     3,     8,   438,   326,     4,   442,   440,   445,
       4,     8,     3,   460,     4,     4,     8,     3,   372,   374,
     626,   636,     3,     8,     4,   573,   561,     4,   564,     4,
       4,   568,   571,     4,     4,   577,   579,   581,   583,     3,
       8,   591,     4,     3,     8,   225,   225,   206,     4,   244,
       4,     4,     4,   209,   209,   209,     4,     4,     4,     4,
       4,     4,   207,   207,   207,   207,   207,   209,   208,   208,
     207,     4,   207,   207,   209,   209,   209,     4,     4,     4,
     209,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,   207,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,   206,   207,   207,   207,
     250,     4,     4,     4,   209,   285,     4,     4,     4,     4,
       4,     4,   207,     4,     4,     4,     4,     4,   207,   207,
     394,     4,   207,   474,     4,     4,     4,     4,     4,     4,
       4,     4,   491,     4,     4,   207,     4,     4,     4,   209,
     433,     4,   209,   209,   457,     4,     4,     4,     4,   370,
     209,     4,     4,   207,     4,   207,   207,     4,     4,   209,
     209,     4,     4,     4,     4,   558,     4,   207,   589,     4,
       7,     7,     7,     7,   206,   206,   206,     7,     7,     5,
     206,   158,   159,   160,   161,   209,   270,   206,   206,     5,
       5,     5,     5,   227,   229,   206,   100,   101,   102,   103,
     411,     5,     5,     5,     5,     7,     7,     7,     7,     7,
     206,   206,     5,     7,   206,   206,     7,     7,     5,   236,
      17,    18,   292,    20,    21,   295,   206,   206,   206,     5,
     206,   206,   236,   206,     7,   206,     5,   206,   236,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   229,   225,   225,   206,   206,   206,    18,
     156,   569,   157,   270,   206,   206,   206,     5,   225,   249,
     588,   284,    24,   301,   302,   303,    34,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,   180,   188,   189,   240,   313,
     314,   315,   318,   320,   322,   324,   325,   327,   328,   329,
     330,   331,   332,   333,   334,   336,   338,   340,   342,   343,
     344,   630,   639,   641,   313,     7,   309,   310,   311,     7,
     387,   388,   389,     7,   425,   426,   427,     7,   449,   450,
     451,     7,   415,   416,   417,   111,   112,   113,   114,   116,
     347,   348,   349,   350,   351,   352,   353,     7,   521,   522,
       7,   483,   484,   485,     7,   363,   364,   365,   124,   125,
     126,   127,   128,   129,   377,   378,   379,   380,   381,   382,
     383,   384,   132,   133,   134,   240,   356,   357,   358,   359,
     360,   477,   479,   136,   137,   240,   477,   479,   534,   535,
     538,   540,   139,   140,   141,   206,   477,   479,   545,   546,
     547,   548,   550,   551,   557,     7,   595,   596,   313,   313,
     182,   240,   477,   479,   536,   537,   538,   540,   542,   237,
       7,   467,   468,   469,   119,   498,   500,   518,   234,   309,
       8,     8,     8,   304,     3,     8,   316,   319,   321,   323,
       4,     4,     4,     4,     4,   335,   337,   339,   341,     4,
       4,     4,     4,     4,     4,     4,   640,   642,     3,     8,
       8,   312,     6,     3,   390,     6,     3,   428,     6,     3,
     452,     6,     3,   418,     6,     3,     3,     6,   523,     3,
       6,   486,     6,     3,   366,     6,     3,     4,     4,     4,
       4,     4,     4,     3,     8,     4,     4,     4,     3,     8,
     539,   541,     3,     8,     4,   549,     4,   552,     3,     8,
       8,   597,     3,     6,     8,     8,     4,     3,     8,   206,
     238,   239,   470,     6,     3,   501,     8,     6,     6,     4,
     302,     4,     4,     4,     4,   207,   209,   207,   209,   207,
       4,     4,     4,     4,   207,   207,   207,   207,   209,   207,
     207,     4,     4,   314,   313,   311,   393,   389,   431,   427,
     455,   451,   240,   251,   252,   253,   254,   255,   256,   257,
     258,   263,   264,   265,   266,   267,   268,   271,   273,   275,
     277,   279,   325,   385,   403,   405,   407,   409,   419,   420,
     447,   477,   479,   492,   494,   496,   516,   417,   348,   107,
     108,   240,   325,   421,   447,   477,   479,   492,   494,   496,
     524,   525,   526,   527,   528,   530,   522,   489,   485,   369,
     365,   207,   207,   207,   207,   207,   207,   378,   209,   207,
     207,   357,     4,     4,   535,   209,     4,   207,     4,   546,
     165,   167,   168,   240,   325,   477,   479,   598,   599,   600,
     601,   603,   596,   209,   537,     6,     3,   473,   469,     4,
     206,    35,    36,    37,    38,   317,   206,   206,   206,   206,
     206,   206,   206,   206,   206,     8,     8,     8,     8,     3,
       8,   529,     4,     8,     3,     8,     8,   206,   206,   206,
     225,   604,     4,   602,     3,     8,   206,     8,   236,   420,
       4,   209,   526,     4,   207,     4,   599,   206,     5,   206,
       7,   605,   606,   607,     3,     6,   166,   169,   170,   171,
     172,   608,   609,   610,   612,   613,   614,   615,   606,   611,
       4,     4,     4,   616,     3,     8,     4,   209,   207,   207,
       4,   609,   206,   206
  };

  const short
  Dhcp4Parser::yyr1_[] =
  {
       0,   210,   212,   211,   213,   211,   214,   211,   215,   211,
     216,   211,   217,   211,   218,   211,   219,   211,   220,   211,
     221,   211,   222,   211,   223,   211,   224,   211,   225,   225,
     225,   225,   225,   225,   225,   226,   228,   227,   229,   230,
     230,   231,   231,   233,   232,   234,   234,   235,   235,   237,
     236,   238,   238,   239,   239,   240,   242,   241,   243,   243,
     244,   246,   245,   248,   247,   249,   249,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   261,   260,   262,
     263,   264,   265,   266,   267,   269,   268,   270,   270,   270,
     270,   270,   272,   271,   274,   273,   276,   275,   278,   277,
     279,   280,   281,   283,   282,   284,   284,   285,   285,   285,
     285,   285,   285,   285,   287,   286,   289,   288,   291,   290,
     292,   292,   294,   293,   295,   295,   296,   298,   297,   300,
     299,   301,   301,   302,   304,   303,   306,   305,   308,   307,
     309,   309,   310,   310,   312,   311,   313,   313,   314,   314,
     314,   314,   314,   314,   314,   314,   314,   314,   314,   314,
     314,   314,   314,   314,   314,   314,   314,   314,   314,   314,
     314,   314,   316,   315,   317,   317,   317,   317,   319,   318,
     321,   320,   323,   322,   324,   326,   325,   327,   328,   329,
     330,   331,   332,   333,   335,   334,   337,   336,   339,   338,
     341,   340,   342,   343,   344,   346,   345,   347,   347,   348,
     348,   348,   348,   348,   349,   350,   351,   352,   353,   355,
     354,   356,   356,   357,   357,   357,   357,   357,   357,   358,
     359,   360,   362,   361,   363,   363,   364,   364,   366,   365,
     368,   367,   369,   369,   369,   370,   370,   370,   370,   372,
     371,   374,   373,   376,   375,   377,   377,   378,   378,   378,
     378,   378,   378,   379,   380,   381,   382,   383,   384,   386,
     385,   387,   387,   388,   388,   390,   389,   392,   391,   393,
     393,   394,   394,   394,   394,   394,   394,   394,   394,   394,
     394,   394,   394,   394,   394,   394,   394,   394,   394,   394,
     394,   394,   394,   394,   394,   394,   394,   394,   394,   394,
     394,   394,   394,   394,   394,   394,   394,   394,   394,   394,
     394,   394,   394,   396,   395,   398,   397,   400,   399,   402,
     401,   404,   403,   406,   405,   408,   407,   410,   409,   411,
     411,   411,   411,   412,   414,   413,   415,   415,   416,   416,
     418,   417,   419,   419,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   420,   420,   420,   422,   421,   424,
     423,   425,   425,   426,   426,   428,   427,   430,   429,   431,
     431,   432,   432,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   434,   435,   436,   438,   437,   440,   439,
     442,   441,   443,   445,   444,   446,   448,   447,   449,   449,
     450,   450,   452,   451,   454,   453,   455,   455,   456,   456,
     457,   457,   457,   457,   457,   457,   457,   457,   457,   458,
     460,   459,   461,   462,   463,   464,   466,   465,   467,   467,
     468,   468,   470,   469,   472,   471,   473,   473,   474,   474,
     474,   474,   474,   474,   474,   474,   474,   476,   475,   478,
     477,   480,   479,   482,   481,   483,   483,   484,   484,   486,
     485,   488,   487,   489,   489,   490,   490,   491,   491,   491,
     491,   491,   491,   491,   491,   491,   491,   491,   491,   491,
     491,   491,   493,   492,   495,   494,   497,   496,   499,   498,
     501,   500,   503,   502,   505,   504,   507,   506,   509,   508,
     511,   510,   513,   512,   515,   514,   517,   516,   518,   518,
     520,   519,   521,   521,   523,   522,   524,   524,   525,   525,
     526,   526,   526,   526,   526,   526,   526,   526,   526,   526,
     526,   527,   529,   528,   530,   531,   533,   532,   534,   534,
     535,   535,   535,   535,   535,   536,   536,   537,   537,   537,
     537,   537,   537,   539,   538,   541,   540,   542,   544,   543,
     545,   545,   546,   546,   546,   546,   546,   546,   547,   549,
     548,   550,   552,   551,   554,   553,   556,   555,   557,   557,
     558,   558,   558,   558,   558,   558,   558,   558,   558,   558,
     558,   558,   558,   558,   558,   558,   558,   558,   559,   561,
     560,   562,   564,   563,   565,   566,   568,   567,   569,   569,
     571,   570,   573,   572,   574,   575,   577,   576,   579,   578,
     581,   580,   583,   582,   585,   584,   587,   586,   588,   588,
     589,   589,   591,   590,   592,   594,   593,   595,   595,   597,
     596,   598,   598,   599,   599,   599,   599,   599,   599,   599,
     600,   602,   601,   604,   603,   605,   605,   607,   606,   608,
     608,   609,   609,   609,   609,   609,   611,   610,   612,   613,
     614,   616,   615,   618,   617,   620,   619,   622,   621,   624,
     623,   626,   625,   628,   627,   629,   630,   632,   631,   633,
     634,   636,   635,   637,   638,   640,   639,   642,   641,   643,
     644,   646,   645
  };

  const signed char
  Dhcp4Parser::yyr2_[] =
  {
       0,     2,     0,     3,     0,     3,     0,     3,     0,     3,
       0,     3,     0,     3,     0,     3,     0,     3,     0,     3,
       0,     3,     0,     3,     0,     3,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     4,     1,     0,
       1,     3,     5,     0,     4,     0,     1,     1,     3,     0,
       4,     0,     1,     1,     3,     2,     0,     4,     1,     3,
       1,     0,     6,     0,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     0,     4,     3,
       3,     3,     3,     3,     3,     0,     4,     1,     1,     1,
       1,     1,     0,     4,     0,     4,     0,     4,     0,     4,
       3,     3,     3,     0,     6,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     0,     4,     0,     4,     0,     4,
       1,     1,     0,     4,     1,     1,     3,     0,     6,     0,
       6,     1,     3,     1,     0,     4,     0,     6,     0,     6,
       0,     1,     1,     3,     0,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     1,     1,     1,     1,     0,     4,
       0,     4,     0,     4,     3,     0,     4,     3,     3,     3,
       3,     3,     3,     3,     0,     4,     0,     4,     0,     4,
       0,     4,     3,     3,     3,     0,     6,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       6,     1,     3,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     0,     6,     0,     1,     1,     3,     0,     4,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     0,
       4,     0,     4,     0,     6,     1,     3,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     0,
       6,     0,     1,     1,     3,     0,     4,     0,     4,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     1,
       1,     1,     1,     3,     0,     6,     0,     1,     1,     3,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     6,     0,
       4,     0,     1,     1,     3,     0,     4,     0,     4,     0,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     0,     4,     0,     4,
       0,     4,     1,     0,     4,     3,     0,     6,     0,     1,
       1,     3,     0,     4,     0,     4,     0,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     4,     1,     1,     3,     3,     0,     6,     0,     1,
       1,     3,     0,     4,     0,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     4,     0,
       4,     0,     4,     0,     6,     0,     1,     1,     3,     0,
       4,     0,     4,     0,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     6,     1,     1,
       0,     6,     1,     3,     0,     4,     0,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     3,     3,     0,     6,     1,     3,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     0,     4,     0,     4,     3,     0,     6,
       1,     3,     1,     1,     1,     1,     1,     1,     3,     0,
       4,     3,     0,     4,     0,     6,     0,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     0,
       4,     3,     0,     4,     3,     3,     0,     4,     1,     1,
       0,     4,     0,     4,     3,     3,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     6,     0,     4,     1,     3,
       1,     1,     0,     6,     3,     0,     6,     1,     3,     0,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       3,     0,     4,     0,     6,     1,     3,     0,     4,     1,
       3,     1,     1,     1,     1,     1,     0,     4,     3,     3,
       3,     0,     4,     0,     4,     0,     6,     0,     4,     0,
       4,     0,     4,     0,     6,     3,     3,     0,     6,     3,
       3,     0,     4,     3,     3,     0,     4,     0,     4,     3,
       3,     0,     6
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const Dhcp4Parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\",\"", "\":\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"null\"", "\"Dhcp4\"", "\"config-control\"",
  "\"config-databases\"", "\"config-fetch-wait-time\"",
  "\"interfaces-config\"", "\"interfaces\"", "\"dhcp-socket-type\"",
  "\"raw\"", "\"udp\"", "\"outbound-interface\"", "\"same-as-inbound\"",
  "\"use-routing\"", "\"re-detect\"", "\"sanity-checks\"",
  "\"lease-checks\"", "\"echo-client-id\"", "\"match-client-id\"",
  "\"authoritative\"", "\"next-server\"", "\"server-hostname\"",
  "\"boot-file-name\"", "\"lease-database\"", "\"hosts-database\"",
  "\"hosts-databases\"", "\"type\"", "\"memfile\"", "\"mysql\"",
  "\"postgresql\"", "\"cql\"", "\"user\"", "\"password\"", "\"host\"",
  "\"port\"", "\"persist\"", "\"lfc-interval\"", "\"readonly\"",
  "\"connect-timeout\"", "\"contact-points\"", "\"keyspace\"",
  "\"consistency\"", "\"serial-consistency\"", "\"max-reconnect-tries\"",
  "\"reconnect-wait-time\"", "\"request-timeout\"", "\"tcp-keepalive\"",
  "\"tcp-nodelay\"", "\"max-row-errors\"", "\"valid-lifetime\"",
  "\"min-valid-lifetime\"", "\"max-valid-lifetime\"", "\"renew-timer\"",
  "\"rebind-timer\"", "\"calculate-tee-times\"", "\"t1-percent\"",
  "\"t2-percent\"", "\"decline-probation-period\"", "\"server-tag\"",
  "\"statistic-default-sample-count\"", "\"statistic-default-sample-age\"",
  "\"ddns-send-updates\"", "\"ddns-override-no-update\"",
  "\"ddns-override-client-update\"", "\"ddns-replace-client-name\"",
  "\"ddns-generated-prefix\"", "\"ddns-qualifying-suffix\"",
  "\"store-extended-info\"", "\"subnet4\"", "\"4o6-interface\"",
  "\"4o6-interface-id\"", "\"4o6-subnet\"", "\"option-def\"",
  "\"option-data\"", "\"name\"", "\"data\"", "\"code\"", "\"space\"",
  "\"csv-format\"", "\"always-send\"", "\"record-types\"",
  "\"encapsulate\"", "\"array\"", "\"shared-networks\"", "\"pools\"",
  "\"pool\"", "\"user-context\"", "\"comment\"", "\"subnet\"",
  "\"interface\"", "\"id\"", "\"reservation-mode\"", "\"disabled\"",
  "\"out-of-pool\"", "\"global\"", "\"all\"",
  "\"host-reservation-identifiers\"", "\"client-classes\"",
  "\"require-client-classes\"", "\"test\"", "\"only-if-required\"",
  "\"client-class\"", "\"reservations\"", "\"duid\"", "\"hw-address\"",
  "\"circuit-id\"", "\"client-id\"", "\"hostname\"", "\"flex-id\"",
  "\"relay\"", "\"ip-address\"", "\"ip-addresses\"", "\"hooks-libraries\"",
  "\"library\"", "\"parameters\"", "\"expired-leases-processing\"",
  "\"reclaim-timer-wait-time\"", "\"flush-reclaimed-timer-wait-time\"",
  "\"hold-reclaimed-time\"", "\"max-reclaim-leases\"",
  "\"max-reclaim-time\"", "\"unwarned-reclaim-cycles\"",
  "\"dhcp4o6-port\"", "\"multi-threading\"", "\"enable-multi-threading\"",
  "\"thread-pool-size\"", "\"packet-queue-size\"", "\"control-socket\"",
  "\"socket-type\"", "\"socket-name\"", "\"dhcp-queue-control\"",
  "\"enable-queue\"", "\"queue-type\"", "\"capacity\"", "\"dhcp-ddns\"",
  "\"enable-updates\"", "\"qualifying-suffix\"", "\"server-ip\"",
  "\"server-port\"", "\"sender-ip\"", "\"sender-port\"",
  "\"max-queue-size\"", "\"ncr-protocol\"", "\"ncr-format\"",
  "\"override-no-update\"", "\"override-client-update\"",
  "\"replace-client-name\"", "\"generated-prefix\"", "\"tcp\"", "\"JSON\"",
  "\"when-present\"", "\"never\"", "\"always\"", "\"when-not-present\"",
  "\"hostname-char-set\"", "\"hostname-char-replacement\"", "\"loggers\"",
  "\"output_options\"", "\"output\"", "\"debuglevel\"", "\"severity\"",
  "\"flush\"", "\"maxsize\"", "\"maxver\"", "\"pattern\"",
  "\"allocation-type\"", "\"config-database\"", "\"configuration-type\"",
  "\"instance-id\"", "\"lawful-interception-parameters\"",
  "\"master-database\"", "\"max-address-utilization\"",
  "\"max-statement-tries\"", "\"notifications\"",
  "\"on-boot-request-configuration-from-kea-netconf\"",
  "\"option-set-description\"", "\"option-set-id\"",
  "\"policy-engine-parameters\"", "\"privacy-history-size\"",
  "\"privacy-valid-lifetime\"", "\"protocol\"", "\"ssl-cert\"",
  "\"v4-excluded-psids\"", "\"v4-psid-len\"", "\"v4-psid-offset\"",
  "TOPLEVEL_JSON", "TOPLEVEL_DHCP4", "SUB_DHCP4", "SUB_INTERFACES4",
  "SUB_SUBNET4", "SUB_POOL4", "SUB_RESERVATION", "SUB_OPTION_DEFS",
  "SUB_OPTION_DEF", "SUB_OPTION_DATA", "SUB_HOOKS_LIBRARY",
  "SUB_DHCP_DDNS", "SUB_CONFIG_CONTROL", "\"constant string\"",
  "\"integer\"", "\"floating point\"", "\"boolean\"", "$accept", "start",
  "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10",
  "$@11", "$@12", "$@13", "value", "sub_json", "map2", "$@14", "map_value",
  "map_content", "not_empty_map", "list_generic", "$@15", "list_content",
  "not_empty_list", "list_strings", "$@16", "list_strings_content",
  "not_empty_list_strings", "unknown_map_entry", "syntax_map", "$@17",
  "global_objects", "global_object", "dhcp4_object", "$@18", "sub_dhcp4",
  "$@19", "global_params", "global_param", "valid_lifetime",
  "min_valid_lifetime", "max_valid_lifetime", "renew_timer",
  "rebind_timer", "calculate_tee_times", "t1_percent", "t2_percent",
  "decline_probation_period", "server_tag", "$@20", "echo_client_id",
  "match_client_id", "authoritative", "ddns_send_updates",
  "ddns_override_no_update", "ddns_override_client_update",
  "ddns_replace_client_name", "$@21", "ddns_replace_client_name_value",
  "ddns_generated_prefix", "$@22", "ddns_qualifying_suffix", "$@23",
  "hostname_char_set", "$@24", "hostname_char_replacement", "$@25",
  "store_extended_info", "statistic_default_sample_count",
  "statistic_default_sample_age", "interfaces_config", "$@26",
  "interfaces_config_params", "interfaces_config_param", "sub_interfaces4",
  "$@27", "interfaces_list", "$@28", "dhcp_socket_type", "$@29",
  "socket_type", "outbound_interface", "$@30", "outbound_interface_value",
  "re_detect", "lease_database", "$@31", "sanity_checks", "$@32",
  "sanity_checks_params", "sanity_checks_param", "lease_checks", "$@33",
  "hosts_database", "$@34", "hosts_databases", "$@35", "database_list",
  "not_empty_database_list", "database", "$@36", "database_map_params",
  "database_map_param", "database_type", "$@37", "db_type", "user", "$@38",
  "password", "$@39", "host", "$@40", "port", "name", "$@41", "persist",
  "lfc_interval", "readonly", "connect_timeout", "request_timeout",
  "tcp_keepalive", "tcp_nodelay", "contact_points", "$@42", "keyspace",
  "$@43", "consistency", "$@44", "serial_consistency", "$@45",
  "max_reconnect_tries", "reconnect_wait_time", "max_row_errors",
  "host_reservation_identifiers", "$@46",
  "host_reservation_identifiers_list", "host_reservation_identifier",
  "duid_id", "hw_address_id", "circuit_id", "client_id", "flex_id",
  "dhcp_multi_threading", "$@47", "multi_threading_params",
  "multi_threading_param", "enable_multi_threading", "thread_pool_size",
  "packet_queue_size", "hooks_libraries", "$@48", "hooks_libraries_list",
  "not_empty_hooks_libraries_list", "hooks_library", "$@49",
  "sub_hooks_library", "$@50", "hooks_params", "hooks_param", "library",
  "$@51", "parameters", "$@52", "expired_leases_processing", "$@53",
  "expired_leases_params", "expired_leases_param",
  "reclaim_timer_wait_time", "flush_reclaimed_timer_wait_time",
  "hold_reclaimed_time", "max_reclaim_leases", "max_reclaim_time",
  "unwarned_reclaim_cycles", "subnet4_list", "$@54",
  "subnet4_list_content", "not_empty_subnet4_list", "subnet4", "$@55",
  "sub_subnet4", "$@56", "subnet4_params", "subnet4_param", "subnet",
  "$@57", "subnet_4o6_interface", "$@58", "subnet_4o6_interface_id",
  "$@59", "subnet_4o6_subnet", "$@60", "interface", "$@61", "client_class",
  "$@62", "require_client_classes", "$@63", "reservation_mode", "$@64",
  "hr_mode", "id", "shared_networks", "$@65", "shared_networks_content",
  "shared_networks_list", "shared_network", "$@66",
  "shared_network_params", "shared_network_param", "option_def_list",
  "$@67", "sub_option_def_list", "$@68", "option_def_list_content",
  "not_empty_option_def_list", "option_def_entry", "$@69",
  "sub_option_def", "$@70", "option_def_params",
  "not_empty_option_def_params", "option_def_param", "option_def_name",
  "code", "option_def_code", "option_def_type", "$@71",
  "option_def_record_types", "$@72", "space", "$@73", "option_def_space",
  "option_def_encapsulate", "$@74", "option_def_array", "option_data_list",
  "$@75", "option_data_list_content", "not_empty_option_data_list",
  "option_data_entry", "$@76", "sub_option_data", "$@77",
  "option_data_params", "not_empty_option_data_params",
  "option_data_param", "option_data_name", "option_data_data", "$@78",
  "option_data_code", "option_data_space", "option_data_csv_format",
  "option_data_always_send", "pools_list", "$@79", "pools_list_content",
  "not_empty_pools_list", "pool_list_entry", "$@80", "sub_pool4", "$@81",
  "pool_params", "pool_param", "pool_entry", "$@82", "user_context",
  "$@83", "comment", "$@84", "reservations", "$@85", "reservations_list",
  "not_empty_reservations_list", "reservation", "$@86", "sub_reservation",
  "$@87", "reservation_params", "not_empty_reservation_params",
  "reservation_param", "next_server", "$@88", "server_hostname", "$@89",
  "boot_file_name", "$@90", "ip_address", "$@91", "ip_addresses", "$@92",
  "duid", "$@93", "hw_address", "$@94", "client_id_value", "$@95",
  "circuit_id_value", "$@96", "flex_id_value", "$@97", "hostname", "$@98",
  "reservation_client_classes", "$@99", "relay", "$@100", "relay_map",
  "client_classes", "$@101", "client_classes_list", "client_class_entry",
  "$@102", "client_class_params", "not_empty_client_class_params",
  "client_class_param", "client_class_name", "client_class_test", "$@103",
  "only_if_required", "dhcp4o6_port", "control_socket", "$@104",
  "control_socket_params", "control_socket_param",
  "notifications_control_socket_params",
  "notifications_control_socket_param", "control_socket_type", "$@105",
  "control_socket_name", "$@106",
  "on-boot-request-configuration-from-kea-netconf", "dhcp_queue_control",
  "$@107", "queue_control_params", "queue_control_param", "enable_queue",
  "queue_type", "$@108", "capacity", "arbitrary_map_entry", "$@109",
  "dhcp_ddns", "$@110", "sub_dhcp_ddns", "$@111", "dhcp_ddns_params",
  "dhcp_ddns_param", "enable_updates", "server_ip", "$@112", "server_port",
  "sender_ip", "$@113", "sender_port", "max_queue_size", "ncr_protocol",
  "$@114", "ncr_protocol_value", "ncr_format", "$@115",
  "dep_qualifying_suffix", "$@116", "dep_override_no_update",
  "dep_override_client_update", "dep_replace_client_name", "$@117",
  "dep_generated_prefix", "$@118", "dep_hostname_char_set", "$@119",
  "dep_hostname_char_replacement", "$@120", "config_control", "$@121",
  "sub_config_control", "$@122", "config_control_params",
  "config_control_param", "config_databases", "$@123",
  "config_fetch_wait_time", "loggers", "$@124", "loggers_entries",
  "logger_entry", "$@125", "logger_params", "logger_param", "debuglevel",
  "severity", "$@126", "output_options_list", "$@127",
  "output_options_list_content", "output_entry", "$@128",
  "output_params_list", "output_params", "output", "$@129", "flush",
  "maxsize", "maxver", "pattern", "$@130", "allocation_type", "$@131",
  "config_database", "$@132", "configuration_type", "$@133", "instance_id",
  "$@134", "lawful-interception-parameters", "$@135", "master_database",
  "$@136", "max_address_utilization", "max_statement_tries",
  "notifications", "$@137", "option_set_description", "option_set_id",
  "policy-engine-parameters", "$@138", "privacy_history_size",
  "privacy_valid_lifetime", "protocol", "$@139", "ssl_cert", "$@140",
  "subnet_v4_psid_offset", "subnet_v4_psid_len",
  "subnet_v4_excluded_psids", "$@141", YY_NULLPTR
  };

#if PARSER4_DEBUG
  const short
  Dhcp4Parser::yyrline_[] =
  {
       0,   299,   299,   299,   300,   300,   301,   301,   302,   302,
     303,   303,   304,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   310,   310,   311,   311,   319,   320,
     321,   322,   323,   324,   325,   328,   333,   333,   344,   347,
     348,   351,   355,   362,   362,   369,   370,   373,   377,   384,
     384,   391,   392,   395,   399,   410,   419,   419,   434,   435,
     439,   442,   442,   459,   459,   468,   469,   474,   475,   476,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,   509,   510,   511,   512,   513,   514,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,   530,   531,   532,   533,   534,   537,   542,
     547,   552,   557,   562,   567,   572,   577,   582,   582,   590,
     595,   600,   605,   610,   615,   620,   620,   628,   631,   634,
     637,   640,   646,   646,   654,   654,   662,   662,   670,   670,
     678,   683,   688,   693,   693,   704,   705,   708,   709,   710,
     711,   712,   713,   714,   717,   717,   726,   726,   736,   736,
     743,   744,   747,   747,   754,   756,   760,   766,   766,   778,
     778,   788,   789,   791,   793,   793,   811,   811,   823,   823,
     833,   834,   837,   838,   841,   841,   851,   852,   855,   856,
     857,   858,   859,   860,   861,   862,   863,   864,   865,   866,
     867,   868,   869,   870,   871,   872,   873,   874,   875,   876,
     877,   878,   881,   881,   888,   889,   890,   891,   894,   894,
     902,   902,   910,   910,   918,   923,   923,   931,   936,   941,
     946,   951,   956,   961,   966,   966,   974,   974,   982,   982,
     990,   990,   998,  1003,  1008,  1014,  1014,  1024,  1025,  1028,
    1029,  1030,  1031,  1032,  1035,  1040,  1045,  1050,  1055,  1062,
    1062,  1074,  1075,  1078,  1079,  1080,  1081,  1082,  1083,  1086,
    1091,  1096,  1101,  1101,  1111,  1112,  1115,  1116,  1119,  1119,
    1129,  1129,  1139,  1140,  1141,  1144,  1145,  1146,  1147,  1150,
    1150,  1158,  1158,  1166,  1166,  1177,  1178,  1181,  1182,  1183,
    1184,  1185,  1186,  1189,  1194,  1199,  1204,  1209,  1214,  1222,
    1222,  1235,  1236,  1239,  1240,  1247,  1247,  1273,  1273,  1284,
    1285,  1289,  1290,  1291,  1292,  1293,  1294,  1295,  1296,  1297,
    1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,  1306,  1307,
    1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,
    1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,  1326,  1327,
    1328,  1329,  1330,  1333,  1333,  1341,  1341,  1349,  1349,  1357,
    1357,  1365,  1365,  1373,  1373,  1381,  1381,  1391,  1391,  1398,
    1399,  1400,  1401,  1404,  1411,  1411,  1422,  1423,  1427,  1428,
    1431,  1431,  1439,  1440,  1443,  1444,  1445,  1446,  1447,  1448,
    1449,  1450,  1451,  1452,  1453,  1454,  1455,  1456,  1457,  1458,
    1459,  1460,  1461,  1462,  1463,  1464,  1465,  1466,  1467,  1468,
    1469,  1470,  1471,  1472,  1473,  1474,  1475,  1482,  1482,  1495,
    1495,  1504,  1505,  1508,  1509,  1514,  1514,  1529,  1529,  1543,
    1544,  1547,  1548,  1551,  1552,  1553,  1554,  1555,  1556,  1557,
    1558,  1559,  1560,  1563,  1565,  1570,  1572,  1572,  1580,  1580,
    1588,  1588,  1596,  1598,  1598,  1606,  1615,  1615,  1627,  1628,
    1633,  1634,  1639,  1639,  1651,  1651,  1663,  1664,  1669,  1670,
    1675,  1676,  1677,  1678,  1679,  1680,  1681,  1682,  1683,  1686,
    1688,  1688,  1696,  1698,  1700,  1705,  1713,  1713,  1725,  1726,
    1729,  1730,  1733,  1733,  1743,  1743,  1753,  1754,  1757,  1758,
    1759,  1760,  1761,  1762,  1763,  1764,  1765,  1768,  1768,  1776,
    1776,  1801,  1801,  1831,  1831,  1841,  1842,  1845,  1846,  1849,
    1849,  1858,  1858,  1867,  1868,  1871,  1872,  1876,  1877,  1878,
    1879,  1880,  1881,  1882,  1883,  1884,  1885,  1886,  1887,  1888,
    1889,  1890,  1893,  1893,  1901,  1901,  1909,  1909,  1917,  1917,
    1925,  1925,  1935,  1935,  1943,  1943,  1951,  1951,  1959,  1959,
    1967,  1967,  1975,  1975,  1983,  1983,  1996,  1996,  2006,  2007,
    2013,  2013,  2023,  2024,  2027,  2027,  2037,  2038,  2041,  2042,
    2045,  2046,  2047,  2048,  2049,  2050,  2051,  2052,  2053,  2054,
    2055,  2058,  2060,  2060,  2068,  2075,  2082,  2082,  2092,  2093,
    2096,  2097,  2098,  2099,  2100,  2103,  2104,  2107,  2108,  2109,
    2110,  2111,  2112,  2115,  2115,  2123,  2123,  2131,  2138,  2138,
    2150,  2151,  2154,  2155,  2156,  2157,  2158,  2159,  2162,  2167,
    2167,  2175,  2180,  2180,  2189,  2189,  2201,  2201,  2211,  2212,
    2215,  2216,  2217,  2218,  2219,  2220,  2221,  2222,  2223,  2224,
    2225,  2226,  2227,  2228,  2229,  2230,  2231,  2232,  2235,  2240,
    2240,  2248,  2253,  2253,  2261,  2266,  2271,  2271,  2279,  2280,
    2283,  2283,  2292,  2292,  2301,  2307,  2313,  2313,  2321,  2321,
    2330,  2330,  2339,  2339,  2350,  2350,  2361,  2361,  2371,  2372,
    2376,  2377,  2380,  2380,  2390,  2397,  2397,  2409,  2410,  2414,
    2414,  2422,  2423,  2426,  2427,  2428,  2429,  2430,  2431,  2432,
    2435,  2440,  2440,  2448,  2448,  2458,  2459,  2462,  2462,  2470,
    2471,  2474,  2475,  2476,  2477,  2478,  2481,  2481,  2489,  2494,
    2499,  2504,  2504,  2514,  2514,  2522,  2522,  2532,  2532,  2540,
    2540,  2548,  2548,  2555,  2555,  2565,  2570,  2575,  2575,  2585,
    2590,  2595,  2595,  2602,  2607,  2612,  2612,  2620,  2620,  2628,
    2633,  2640,  2640
  };

  // Print the state stack on the debug stream.
  void
  Dhcp4Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Dhcp4Parser::yy_reduce_print_ (int yyrule)
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // PARSER4_DEBUG


#line 14 "dhcp4_parser.yy"
} } // isc::dhcp
#line 5760 "dhcp4_parser.cc"

#line 2650 "dhcp4_parser.yy"


void
isc::dhcp::Dhcp4Parser::error(const location_type& loc,
                              const std::string& what)
{
    ctx.error(loc, what);
}
