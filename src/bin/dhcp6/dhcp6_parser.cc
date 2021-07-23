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
#define yylex   parser6_lex



#include "dhcp6_parser.h"


// Unqualified %code blocks.
#line 34 "dhcp6_parser.yy"

#include <dhcp6/parser_context.h>

#line 51 "dhcp6_parser.cc"


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
#if PARSER6_DEBUG

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

#else // !PARSER6_DEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !PARSER6_DEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 14 "dhcp6_parser.yy"
namespace isc { namespace dhcp {
#line 143 "dhcp6_parser.cc"


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Dhcp6Parser::yytnamerr_ (const char *yystr)
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
  Dhcp6Parser::Dhcp6Parser (isc::dhcp::Parser6Context& ctx_yyarg)
#if PARSER6_DEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      ctx (ctx_yyarg)
  {}

  Dhcp6Parser::~Dhcp6Parser ()
  {}

  Dhcp6Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  Dhcp6Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Dhcp6Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Dhcp6Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Dhcp6Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Dhcp6Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Dhcp6Parser::symbol_number_type
  Dhcp6Parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  Dhcp6Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Dhcp6Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 233: // value
      case 237: // map_value
      case 278: // ddns_replace_client_name_value
      case 315: // db_type
      case 418: // hr_mode
      case 554: // duid_type
      case 600: // ncr_protocol_value
        value.YY_MOVE_OR_COPY< ElementPtr > (YY_MOVE (that.value));
        break;

      case 216: // "boolean"
        value.YY_MOVE_OR_COPY< bool > (YY_MOVE (that.value));
        break;

      case 215: // "floating point"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case 214: // "integer"
        value.YY_MOVE_OR_COPY< int64_t > (YY_MOVE (that.value));
        break;

      case 213: // "constant string"
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

  Dhcp6Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 233: // value
      case 237: // map_value
      case 278: // ddns_replace_client_name_value
      case 315: // db_type
      case 418: // hr_mode
      case 554: // duid_type
      case 600: // ncr_protocol_value
        value.move< ElementPtr > (YY_MOVE (that.value));
        break;

      case 216: // "boolean"
        value.move< bool > (YY_MOVE (that.value));
        break;

      case 215: // "floating point"
        value.move< double > (YY_MOVE (that.value));
        break;

      case 214: // "integer"
        value.move< int64_t > (YY_MOVE (that.value));
        break;

      case 213: // "constant string"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  Dhcp6Parser::stack_symbol_type&
  Dhcp6Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 233: // value
      case 237: // map_value
      case 278: // ddns_replace_client_name_value
      case 315: // db_type
      case 418: // hr_mode
      case 554: // duid_type
      case 600: // ncr_protocol_value
        value.copy< ElementPtr > (that.value);
        break;

      case 216: // "boolean"
        value.copy< bool > (that.value);
        break;

      case 215: // "floating point"
        value.copy< double > (that.value);
        break;

      case 214: // "integer"
        value.copy< int64_t > (that.value);
        break;

      case 213: // "constant string"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  Dhcp6Parser::stack_symbol_type&
  Dhcp6Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 233: // value
      case 237: // map_value
      case 278: // ddns_replace_client_name_value
      case 315: // db_type
      case 418: // hr_mode
      case 554: // duid_type
      case 600: // ncr_protocol_value
        value.move< ElementPtr > (that.value);
        break;

      case 216: // "boolean"
        value.move< bool > (that.value);
        break;

      case 215: // "floating point"
        value.move< double > (that.value);
        break;

      case 214: // "integer"
        value.move< int64_t > (that.value);
        break;

      case 213: // "constant string"
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
  Dhcp6Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if PARSER6_DEBUG
  template <typename Base>
  void
  Dhcp6Parser::yy_print_ (std::ostream& yyo,
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
      case 213: // "constant string"
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < std::string > (); }
#line 443 "dhcp6_parser.cc"
        break;

      case 214: // "integer"
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < int64_t > (); }
#line 449 "dhcp6_parser.cc"
        break;

      case 215: // "floating point"
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < double > (); }
#line 455 "dhcp6_parser.cc"
        break;

      case 216: // "boolean"
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < bool > (); }
#line 461 "dhcp6_parser.cc"
        break;

      case 233: // value
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 467 "dhcp6_parser.cc"
        break;

      case 237: // map_value
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 473 "dhcp6_parser.cc"
        break;

      case 278: // ddns_replace_client_name_value
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 479 "dhcp6_parser.cc"
        break;

      case 315: // db_type
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 485 "dhcp6_parser.cc"
        break;

      case 418: // hr_mode
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 491 "dhcp6_parser.cc"
        break;

      case 554: // duid_type
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 497 "dhcp6_parser.cc"
        break;

      case 600: // ncr_protocol_value
#line 295 "dhcp6_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 503 "dhcp6_parser.cc"
        break;

      default:
        break;
    }
    yyo << ')';
  }
#endif

  void
  Dhcp6Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Dhcp6Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Dhcp6Parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if PARSER6_DEBUG
  std::ostream&
  Dhcp6Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Dhcp6Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Dhcp6Parser::debug_level_type
  Dhcp6Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Dhcp6Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // PARSER6_DEBUG

  Dhcp6Parser::state_type
  Dhcp6Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  Dhcp6Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Dhcp6Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Dhcp6Parser::operator() ()
  {
    return parse ();
  }

  int
  Dhcp6Parser::parse ()
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
      case 233: // value
      case 237: // map_value
      case 278: // ddns_replace_client_name_value
      case 315: // db_type
      case 418: // hr_mode
      case 554: // duid_type
      case 600: // ncr_protocol_value
        yylhs.value.emplace< ElementPtr > ();
        break;

      case 216: // "boolean"
        yylhs.value.emplace< bool > ();
        break;

      case 215: // "floating point"
        yylhs.value.emplace< double > ();
        break;

      case 214: // "integer"
        yylhs.value.emplace< int64_t > ();
        break;

      case 213: // "constant string"
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
#line 304 "dhcp6_parser.yy"
                     { ctx.ctx_ = ctx.NO_KEYWORD; }
#line 770 "dhcp6_parser.cc"
    break;

  case 4:
#line 305 "dhcp6_parser.yy"
                      { ctx.ctx_ = ctx.CONFIG; }
#line 776 "dhcp6_parser.cc"
    break;

  case 6:
#line 306 "dhcp6_parser.yy"
                 { ctx.ctx_ = ctx.DHCP6; }
#line 782 "dhcp6_parser.cc"
    break;

  case 8:
#line 307 "dhcp6_parser.yy"
                       { ctx.ctx_ = ctx.INTERFACES_CONFIG; }
#line 788 "dhcp6_parser.cc"
    break;

  case 10:
#line 308 "dhcp6_parser.yy"
                   { ctx.ctx_ = ctx.SUBNET6; }
#line 794 "dhcp6_parser.cc"
    break;

  case 12:
#line 309 "dhcp6_parser.yy"
                 { ctx.ctx_ = ctx.POOLS; }
#line 800 "dhcp6_parser.cc"
    break;

  case 14:
#line 310 "dhcp6_parser.yy"
                   { ctx.ctx_ = ctx.PD_POOLS; }
#line 806 "dhcp6_parser.cc"
    break;

  case 16:
#line 311 "dhcp6_parser.yy"
                       { ctx.ctx_ = ctx.RESERVATIONS; }
#line 812 "dhcp6_parser.cc"
    break;

  case 18:
#line 312 "dhcp6_parser.yy"
                       { ctx.ctx_ = ctx.DHCP6; }
#line 818 "dhcp6_parser.cc"
    break;

  case 20:
#line 313 "dhcp6_parser.yy"
                      { ctx.ctx_ = ctx.OPTION_DEF; }
#line 824 "dhcp6_parser.cc"
    break;

  case 22:
#line 314 "dhcp6_parser.yy"
                       { ctx.ctx_ = ctx.OPTION_DATA; }
#line 830 "dhcp6_parser.cc"
    break;

  case 24:
#line 315 "dhcp6_parser.yy"
                         { ctx.ctx_ = ctx.HOOKS_LIBRARIES; }
#line 836 "dhcp6_parser.cc"
    break;

  case 26:
#line 316 "dhcp6_parser.yy"
                     { ctx.ctx_ = ctx.DHCP_DDNS; }
#line 842 "dhcp6_parser.cc"
    break;

  case 28:
#line 317 "dhcp6_parser.yy"
                          { ctx.ctx_ = ctx.CONFIG_CONTROL; }
#line 848 "dhcp6_parser.cc"
    break;

  case 30:
#line 325 "dhcp6_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location))); }
#line 854 "dhcp6_parser.cc"
    break;

  case 31:
#line 326 "dhcp6_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location))); }
#line 860 "dhcp6_parser.cc"
    break;

  case 32:
#line 327 "dhcp6_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location))); }
#line 866 "dhcp6_parser.cc"
    break;

  case 33:
#line 328 "dhcp6_parser.yy"
              { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location))); }
#line 872 "dhcp6_parser.cc"
    break;

  case 34:
#line 329 "dhcp6_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new NullElement(ctx.loc2pos(yystack_[0].location))); }
#line 878 "dhcp6_parser.cc"
    break;

  case 35:
#line 330 "dhcp6_parser.yy"
            { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 884 "dhcp6_parser.cc"
    break;

  case 36:
#line 331 "dhcp6_parser.yy"
                    { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 890 "dhcp6_parser.cc"
    break;

  case 37:
#line 334 "dhcp6_parser.yy"
                {
    // Push back the JSON value on the stack
    ctx.stack_.push_back(yystack_[0].value.as < ElementPtr > ());
}
#line 899 "dhcp6_parser.cc"
    break;

  case 38:
#line 339 "dhcp6_parser.yy"
                     {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 910 "dhcp6_parser.cc"
    break;

  case 39:
#line 344 "dhcp6_parser.yy"
                             {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
}
#line 920 "dhcp6_parser.cc"
    break;

  case 40:
#line 350 "dhcp6_parser.yy"
                { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 926 "dhcp6_parser.cc"
    break;

  case 43:
#line 357 "dhcp6_parser.yy"
                                  {
                  // map containing a single entry
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 935 "dhcp6_parser.cc"
    break;

  case 44:
#line 361 "dhcp6_parser.yy"
                                                      {
                  // map consisting of a shorter map followed by
                  // comma and string:value
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 945 "dhcp6_parser.cc"
    break;

  case 45:
#line 368 "dhcp6_parser.yy"
                              {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(l);
}
#line 954 "dhcp6_parser.cc"
    break;

  case 46:
#line 371 "dhcp6_parser.yy"
                               {
    // list parsing complete. Put any sanity checking here
}
#line 962 "dhcp6_parser.cc"
    break;

  case 49:
#line 379 "dhcp6_parser.yy"
                      {
                  // List consisting of a single element.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 971 "dhcp6_parser.cc"
    break;

  case 50:
#line 383 "dhcp6_parser.yy"
                                           {
                  // List ending with , and a value.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 980 "dhcp6_parser.cc"
    break;

  case 51:
#line 390 "dhcp6_parser.yy"
                              {
    // List parsing about to start
}
#line 988 "dhcp6_parser.cc"
    break;

  case 52:
#line 392 "dhcp6_parser.yy"
                                       {
    // list parsing complete. Put any sanity checking here
    //ctx.stack_.pop_back();
}
#line 997 "dhcp6_parser.cc"
    break;

  case 55:
#line 401 "dhcp6_parser.yy"
                               {
                          ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
                          ctx.stack_.back()->add(s);
                          }
#line 1006 "dhcp6_parser.cc"
    break;

  case 56:
#line 405 "dhcp6_parser.yy"
                                                            {
                          ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
                          ctx.stack_.back()->add(s);
                          }
#line 1015 "dhcp6_parser.cc"
    break;

  case 57:
#line 416 "dhcp6_parser.yy"
                                {
    const std::string& where = ctx.contextName();
    const std::string& keyword = yystack_[1].value.as < std::string > ();
    error(yystack_[1].location,
          "got unexpected keyword \"" + keyword + "\" in " + where + " map.");
}
#line 1026 "dhcp6_parser.cc"
    break;

  case 58:
#line 425 "dhcp6_parser.yy"
                           {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 1037 "dhcp6_parser.cc"
    break;

  case 59:
#line 430 "dhcp6_parser.yy"
                                {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.

    // Dhcp6 is required
    ctx.require("Dhcp6", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
}
#line 1050 "dhcp6_parser.cc"
    break;

  case 63:
#line 448 "dhcp6_parser.yy"
                    {
    // This code is executed when we're about to start parsing
    // the content of the map
    // Prevent against duplicate.
    ctx.unique("Dhcp6", ctx.loc2pos(yystack_[0].location));
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("Dhcp6", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.DHCP6);
}
#line 1065 "dhcp6_parser.cc"
    break;

  case 64:
#line 457 "dhcp6_parser.yy"
                                                    {
    // No global parameter is required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1075 "dhcp6_parser.cc"
    break;

  case 65:
#line 465 "dhcp6_parser.yy"
                          {
    // Parse the Dhcp6 map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 1085 "dhcp6_parser.cc"
    break;

  case 66:
#line 469 "dhcp6_parser.yy"
                               {
    // No global parameter is required
    // parsing completed
}
#line 1094 "dhcp6_parser.cc"
    break;

  case 131:
#line 544 "dhcp6_parser.yy"
                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1102 "dhcp6_parser.cc"
    break;

  case 132:
#line 546 "dhcp6_parser.yy"
               {
    ElementPtr datadir(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("data-directory", datadir);
    ctx.leave();
}
#line 1112 "dhcp6_parser.cc"
    break;

  case 133:
#line 552 "dhcp6_parser.yy"
                                                     {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("preferred-lifetime", prf);
}
#line 1121 "dhcp6_parser.cc"
    break;

  case 134:
#line 557 "dhcp6_parser.yy"
                                                             {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("min-preferred-lifetime", prf);
}
#line 1130 "dhcp6_parser.cc"
    break;

  case 135:
#line 562 "dhcp6_parser.yy"
                                                             {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-preferred-lifetime", prf);
}
#line 1139 "dhcp6_parser.cc"
    break;

  case 136:
#line 567 "dhcp6_parser.yy"
                                             {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("valid-lifetime", prf);
}
#line 1148 "dhcp6_parser.cc"
    break;

  case 137:
#line 572 "dhcp6_parser.yy"
                                                     {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("min-valid-lifetime", prf);
}
#line 1157 "dhcp6_parser.cc"
    break;

  case 138:
#line 577 "dhcp6_parser.yy"
                                                     {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-valid-lifetime", prf);
}
#line 1166 "dhcp6_parser.cc"
    break;

  case 139:
#line 582 "dhcp6_parser.yy"
                                       {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("renew-timer", prf);
}
#line 1175 "dhcp6_parser.cc"
    break;

  case 140:
#line 587 "dhcp6_parser.yy"
                                         {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("rebind-timer", prf);
}
#line 1184 "dhcp6_parser.cc"
    break;

  case 141:
#line 592 "dhcp6_parser.yy"
                                                       {
    ElementPtr ctt(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("calculate-tee-times", ctt);
}
#line 1193 "dhcp6_parser.cc"
    break;

  case 142:
#line 597 "dhcp6_parser.yy"
                                   {
    ElementPtr t1(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("t1-percent", t1);
}
#line 1202 "dhcp6_parser.cc"
    break;

  case 143:
#line 602 "dhcp6_parser.yy"
                                   {
    ElementPtr t2(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("t2-percent", t2);
}
#line 1211 "dhcp6_parser.cc"
    break;

  case 144:
#line 607 "dhcp6_parser.yy"
                                                                 {
    ElementPtr dpp(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("decline-probation-period", dpp);
}
#line 1220 "dhcp6_parser.cc"
    break;

  case 145:
#line 612 "dhcp6_parser.yy"
                                                   {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-send-updates", b);
}
#line 1229 "dhcp6_parser.cc"
    break;

  case 146:
#line 617 "dhcp6_parser.yy"
                                                               {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-override-no-update", b);
}
#line 1238 "dhcp6_parser.cc"
    break;

  case 147:
#line 622 "dhcp6_parser.yy"
                                                                       {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-override-client-update", b);
}
#line 1247 "dhcp6_parser.cc"
    break;

  case 148:
#line 627 "dhcp6_parser.yy"
                                                   {
    ctx.enter(ctx.REPLACE_CLIENT_NAME);
}
#line 1255 "dhcp6_parser.cc"
    break;

  case 149:
#line 629 "dhcp6_parser.yy"
                                       {
    ctx.stack_.back()->set("ddns-replace-client-name", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1264 "dhcp6_parser.cc"
    break;

  case 150:
#line 635 "dhcp6_parser.yy"
                 {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("when-present", ctx.loc2pos(yystack_[0].location)));
      }
#line 1272 "dhcp6_parser.cc"
    break;

  case 151:
#line 638 "dhcp6_parser.yy"
          {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("never", ctx.loc2pos(yystack_[0].location)));
      }
#line 1280 "dhcp6_parser.cc"
    break;

  case 152:
#line 641 "dhcp6_parser.yy"
           {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("always", ctx.loc2pos(yystack_[0].location)));
      }
#line 1288 "dhcp6_parser.cc"
    break;

  case 153:
#line 644 "dhcp6_parser.yy"
                     {
      yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("when-not-present", ctx.loc2pos(yystack_[0].location)));
      }
#line 1296 "dhcp6_parser.cc"
    break;

  case 154:
#line 647 "dhcp6_parser.yy"
            {
      error(yystack_[0].location, "boolean values for the replace-client-name are "
                "no longer supported");
      }
#line 1305 "dhcp6_parser.cc"
    break;

  case 155:
#line 653 "dhcp6_parser.yy"
                                             {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1313 "dhcp6_parser.cc"
    break;

  case 156:
#line 655 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-generated-prefix", s);
    ctx.leave();
}
#line 1323 "dhcp6_parser.cc"
    break;

  case 157:
#line 661 "dhcp6_parser.yy"
                                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1331 "dhcp6_parser.cc"
    break;

  case 158:
#line 663 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ddns-qualifying-suffix", s);
    ctx.leave();
}
#line 1341 "dhcp6_parser.cc"
    break;

  case 159:
#line 669 "dhcp6_parser.yy"
                                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1349 "dhcp6_parser.cc"
    break;

  case 160:
#line 671 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-set", s);
    ctx.leave();
}
#line 1359 "dhcp6_parser.cc"
    break;

  case 161:
#line 677 "dhcp6_parser.yy"
                                                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1367 "dhcp6_parser.cc"
    break;

  case 162:
#line 679 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-replacement", s);
    ctx.leave();
}
#line 1377 "dhcp6_parser.cc"
    break;

  case 163:
#line 685 "dhcp6_parser.yy"
                                                       {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("store-extended-info", b);
}
#line 1386 "dhcp6_parser.cc"
    break;

  case 164:
#line 690 "dhcp6_parser.yy"
                                                                             {
    ElementPtr count(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("statistic-default-sample-count", count);
}
#line 1395 "dhcp6_parser.cc"
    break;

  case 165:
#line 695 "dhcp6_parser.yy"
                                                                         {
    ElementPtr age(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("statistic-default-sample-age", age);
}
#line 1404 "dhcp6_parser.cc"
    break;

  case 166:
#line 700 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1412 "dhcp6_parser.cc"
    break;

  case 167:
#line 702 "dhcp6_parser.yy"
               {
    ElementPtr stag(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-tag", stag);
    ctx.leave();
}
#line 1422 "dhcp6_parser.cc"
    break;

  case 168:
#line 708 "dhcp6_parser.yy"
                                     {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interfaces-config", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.INTERFACES_CONFIG);
}
#line 1433 "dhcp6_parser.cc"
    break;

  case 169:
#line 713 "dhcp6_parser.yy"
                                                               {
    // No interfaces config param is required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1443 "dhcp6_parser.cc"
    break;

  case 170:
#line 719 "dhcp6_parser.yy"
                                {
    // Parse the interfaces-config map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 1453 "dhcp6_parser.cc"
    break;

  case 171:
#line 723 "dhcp6_parser.yy"
                                          {
    // No interfaces config param is required
    // parsing completed
}
#line 1462 "dhcp6_parser.cc"
    break;

  case 179:
#line 739 "dhcp6_parser.yy"
                            {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interfaces", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1473 "dhcp6_parser.cc"
    break;

  case 180:
#line 744 "dhcp6_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1482 "dhcp6_parser.cc"
    break;

  case 181:
#line 749 "dhcp6_parser.yy"
                                   {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("re-detect", b);
}
#line 1491 "dhcp6_parser.cc"
    break;

  case 182:
#line 754 "dhcp6_parser.yy"
                               {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("lease-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.LEASE_DATABASE);
}
#line 1502 "dhcp6_parser.cc"
    break;

  case 183:
#line 759 "dhcp6_parser.yy"
                                                          {
    // The type parameter is required
    ctx.require("type", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1513 "dhcp6_parser.cc"
    break;

  case 184:
#line 766 "dhcp6_parser.yy"
                               {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hosts-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.HOSTS_DATABASE);
}
#line 1524 "dhcp6_parser.cc"
    break;

  case 185:
#line 771 "dhcp6_parser.yy"
                                                          {
    // The type parameter is required
    ctx.require("type", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1535 "dhcp6_parser.cc"
    break;

  case 186:
#line 778 "dhcp6_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hosts-databases", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOSTS_DATABASE);
}
#line 1546 "dhcp6_parser.cc"
    break;

  case 187:
#line 783 "dhcp6_parser.yy"
                                                      {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1555 "dhcp6_parser.cc"
    break;

  case 192:
#line 796 "dhcp6_parser.yy"
                         {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1565 "dhcp6_parser.cc"
    break;

  case 193:
#line 800 "dhcp6_parser.yy"
                                     {
    // The type parameter is required
    ctx.require("type", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 1575 "dhcp6_parser.cc"
    break;

  case 220:
#line 836 "dhcp6_parser.yy"
                    {
    ctx.enter(ctx.DATABASE_TYPE);
}
#line 1583 "dhcp6_parser.cc"
    break;

  case 221:
#line 838 "dhcp6_parser.yy"
                {
    ctx.stack_.back()->set("type", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1592 "dhcp6_parser.cc"
    break;

  case 222:
#line 843 "dhcp6_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("memfile", ctx.loc2pos(yystack_[0].location))); }
#line 1598 "dhcp6_parser.cc"
    break;

  case 223:
#line 844 "dhcp6_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("mysql", ctx.loc2pos(yystack_[0].location))); }
#line 1604 "dhcp6_parser.cc"
    break;

  case 224:
#line 845 "dhcp6_parser.yy"
                    { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("postgresql", ctx.loc2pos(yystack_[0].location))); }
#line 1610 "dhcp6_parser.cc"
    break;

  case 225:
#line 846 "dhcp6_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("cql", ctx.loc2pos(yystack_[0].location))); }
#line 1616 "dhcp6_parser.cc"
    break;

  case 226:
#line 849 "dhcp6_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1624 "dhcp6_parser.cc"
    break;

  case 227:
#line 851 "dhcp6_parser.yy"
               {
    ElementPtr user(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("user", user);
    ctx.leave();
}
#line 1634 "dhcp6_parser.cc"
    break;

  case 228:
#line 857 "dhcp6_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1642 "dhcp6_parser.cc"
    break;

  case 229:
#line 859 "dhcp6_parser.yy"
               {
    ElementPtr pwd(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("password", pwd);
    ctx.leave();
}
#line 1652 "dhcp6_parser.cc"
    break;

  case 230:
#line 865 "dhcp6_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1660 "dhcp6_parser.cc"
    break;

  case 231:
#line 867 "dhcp6_parser.yy"
               {
    ElementPtr h(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("host", h);
    ctx.leave();
}
#line 1670 "dhcp6_parser.cc"
    break;

  case 232:
#line 873 "dhcp6_parser.yy"
                         {
    ElementPtr p(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("port", p);
}
#line 1679 "dhcp6_parser.cc"
    break;

  case 233:
#line 878 "dhcp6_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1687 "dhcp6_parser.cc"
    break;

  case 234:
#line 880 "dhcp6_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("name", name);
    ctx.leave();
}
#line 1697 "dhcp6_parser.cc"
    break;

  case 235:
#line 886 "dhcp6_parser.yy"
                               {
    ElementPtr n(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("persist", n);
}
#line 1706 "dhcp6_parser.cc"
    break;

  case 236:
#line 891 "dhcp6_parser.yy"
                                         {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("lfc-interval", n);
}
#line 1715 "dhcp6_parser.cc"
    break;

  case 237:
#line 896 "dhcp6_parser.yy"
                                 {
    ElementPtr n(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("readonly", n);
}
#line 1724 "dhcp6_parser.cc"
    break;

  case 238:
#line 901 "dhcp6_parser.yy"
                                               {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("connect-timeout", n);
}
#line 1733 "dhcp6_parser.cc"
    break;

  case 239:
#line 906 "dhcp6_parser.yy"
                                                       {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("reconnect-wait-time", n);
}
#line 1742 "dhcp6_parser.cc"
    break;

  case 240:
#line 911 "dhcp6_parser.yy"
                                             {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-row-errors", n);
}
#line 1751 "dhcp6_parser.cc"
    break;

  case 241:
#line 916 "dhcp6_parser.yy"
                                               {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("request-timeout", n);
}
#line 1760 "dhcp6_parser.cc"
    break;

  case 242:
#line 921 "dhcp6_parser.yy"
                                           {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("tcp-keepalive", n);
}
#line 1769 "dhcp6_parser.cc"
    break;

  case 243:
#line 926 "dhcp6_parser.yy"
                                       {
    ElementPtr n(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("tcp-nodelay", n);
}
#line 1778 "dhcp6_parser.cc"
    break;

  case 244:
#line 931 "dhcp6_parser.yy"
                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1786 "dhcp6_parser.cc"
    break;

  case 245:
#line 933 "dhcp6_parser.yy"
               {
    ElementPtr cp(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("contact-points", cp);
    ctx.leave();
}
#line 1796 "dhcp6_parser.cc"
    break;

  case 246:
#line 939 "dhcp6_parser.yy"
                                                       {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-reconnect-tries", n);
}
#line 1805 "dhcp6_parser.cc"
    break;

  case 247:
#line 944 "dhcp6_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1813 "dhcp6_parser.cc"
    break;

  case 248:
#line 946 "dhcp6_parser.yy"
               {
    ElementPtr ks(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("keyspace", ks);
    ctx.leave();
}
#line 1823 "dhcp6_parser.cc"
    break;

  case 249:
#line 952 "dhcp6_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1831 "dhcp6_parser.cc"
    break;

  case 250:
#line 954 "dhcp6_parser.yy"
               {
    ElementPtr c(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("consistency", c);
    ctx.leave();
}
#line 1841 "dhcp6_parser.cc"
    break;

  case 251:
#line 960 "dhcp6_parser.yy"
                                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1849 "dhcp6_parser.cc"
    break;

  case 252:
#line 962 "dhcp6_parser.yy"
               {
    ElementPtr c(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("serial-consistency", c);
    ctx.leave();
}
#line 1859 "dhcp6_parser.cc"
    break;

  case 253:
#line 968 "dhcp6_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("sanity-checks", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.SANITY_CHECKS);
}
#line 1870 "dhcp6_parser.cc"
    break;

  case 254:
#line 973 "dhcp6_parser.yy"
                                                           {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1879 "dhcp6_parser.cc"
    break;

  case 258:
#line 983 "dhcp6_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1887 "dhcp6_parser.cc"
    break;

  case 259:
#line 985 "dhcp6_parser.yy"
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
#line 1907 "dhcp6_parser.cc"
    break;

  case 260:
#line 1001 "dhcp6_parser.yy"
                         {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("mac-sources", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.MAC_SOURCES);
}
#line 1918 "dhcp6_parser.cc"
    break;

  case 261:
#line 1006 "dhcp6_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1927 "dhcp6_parser.cc"
    break;

  case 266:
#line 1019 "dhcp6_parser.yy"
               {
    ElementPtr duid(new StringElement("duid", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(duid);
}
#line 1936 "dhcp6_parser.cc"
    break;

  case 267:
#line 1024 "dhcp6_parser.yy"
                   {
    ElementPtr duid(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(duid);
}
#line 1945 "dhcp6_parser.cc"
    break;

  case 268:
#line 1029 "dhcp6_parser.yy"
                                                           {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("host-reservation-identifiers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOST_RESERVATION_IDENTIFIERS);
}
#line 1956 "dhcp6_parser.cc"
    break;

  case 269:
#line 1034 "dhcp6_parser.yy"
                                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1965 "dhcp6_parser.cc"
    break;

  case 275:
#line 1048 "dhcp6_parser.yy"
                           {
    ElementPtr hwaddr(new StringElement("hw-address", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(hwaddr);
}
#line 1974 "dhcp6_parser.cc"
    break;

  case 276:
#line 1053 "dhcp6_parser.yy"
                  {
    ElementPtr flex_id(new StringElement("flex-id", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(flex_id);
}
#line 1983 "dhcp6_parser.cc"
    break;

  case 277:
#line 1060 "dhcp6_parser.yy"
                                               {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("relay-supplied-options", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 1994 "dhcp6_parser.cc"
    break;

  case 278:
#line 1065 "dhcp6_parser.yy"
                                                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2003 "dhcp6_parser.cc"
    break;

  case 279:
#line 1072 "dhcp6_parser.yy"
                                           {
    ElementPtr mt(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("multi-threading", mt);
    ctx.stack_.push_back(mt);
    ctx.enter(ctx.DHCP_MULTI_THREADING);
}
#line 2014 "dhcp6_parser.cc"
    break;

  case 280:
#line 1077 "dhcp6_parser.yy"
                                                             {
    // The enable parameter is required.
    ctx.require("enable-multi-threading", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2025 "dhcp6_parser.cc"
    break;

  case 289:
#line 1096 "dhcp6_parser.yy"
                                                             {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enable-multi-threading", b);
}
#line 2034 "dhcp6_parser.cc"
    break;

  case 290:
#line 1101 "dhcp6_parser.yy"
                                                 {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("thread-pool-size", prf);
}
#line 2043 "dhcp6_parser.cc"
    break;

  case 291:
#line 1106 "dhcp6_parser.yy"
                                                   {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("packet-queue-size", prf);
}
#line 2052 "dhcp6_parser.cc"
    break;

  case 292:
#line 1111 "dhcp6_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hooks-libraries", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOOKS_LIBRARIES);
}
#line 2063 "dhcp6_parser.cc"
    break;

  case 293:
#line 1116 "dhcp6_parser.yy"
                                                             {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2072 "dhcp6_parser.cc"
    break;

  case 298:
#line 1129 "dhcp6_parser.yy"
                              {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2082 "dhcp6_parser.cc"
    break;

  case 299:
#line 1133 "dhcp6_parser.yy"
                              {
    // The library hooks parameter is required
    ctx.require("library", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("library-path", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2093 "dhcp6_parser.cc"
    break;

  case 300:
#line 1140 "dhcp6_parser.yy"
                                  {
    // Parse the hooks-libraries list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2103 "dhcp6_parser.cc"
    break;

  case 301:
#line 1144 "dhcp6_parser.yy"
                              {
    // The library hooks parameter is required
    ctx.require("library", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("library-path", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2114 "dhcp6_parser.cc"
    break;

  case 310:
#line 1163 "dhcp6_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2122 "dhcp6_parser.cc"
    break;

  case 311:
#line 1165 "dhcp6_parser.yy"
               {
    ElementPtr lib(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("library", lib);
    ctx.leave();
}
#line 2132 "dhcp6_parser.cc"
    break;

  case 312:
#line 1171 "dhcp6_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2140 "dhcp6_parser.cc"
    break;

  case 313:
#line 1173 "dhcp6_parser.yy"
               {
    ElementPtr lib(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("library-path", lib);
    ctx.leave();
}
#line 2150 "dhcp6_parser.cc"
    break;

  case 314:
#line 1179 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2158 "dhcp6_parser.cc"
    break;

  case 315:
#line 1181 "dhcp6_parser.yy"
                  {
    ctx.stack_.back()->set("parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 2167 "dhcp6_parser.cc"
    break;

  case 316:
#line 1187 "dhcp6_parser.yy"
                                                     {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("expired-leases-processing", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.EXPIRED_LEASES_PROCESSING);
}
#line 2178 "dhcp6_parser.cc"
    break;

  case 317:
#line 1192 "dhcp6_parser.yy"
                                                            {
    // No expired lease parameter is required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2188 "dhcp6_parser.cc"
    break;

  case 326:
#line 1210 "dhcp6_parser.yy"
                                                               {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("reclaim-timer-wait-time", value);
}
#line 2197 "dhcp6_parser.cc"
    break;

  case 327:
#line 1215 "dhcp6_parser.yy"
                                                                               {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flush-reclaimed-timer-wait-time", value);
}
#line 2206 "dhcp6_parser.cc"
    break;

  case 328:
#line 1220 "dhcp6_parser.yy"
                                                       {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hold-reclaimed-time", value);
}
#line 2215 "dhcp6_parser.cc"
    break;

  case 329:
#line 1225 "dhcp6_parser.yy"
                                                     {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-reclaim-leases", value);
}
#line 2224 "dhcp6_parser.cc"
    break;

  case 330:
#line 1230 "dhcp6_parser.yy"
                                                 {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-reclaim-time", value);
}
#line 2233 "dhcp6_parser.cc"
    break;

  case 331:
#line 1235 "dhcp6_parser.yy"
                                                               {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("unwarned-reclaim-cycles", value);
}
#line 2242 "dhcp6_parser.cc"
    break;

  case 332:
#line 1243 "dhcp6_parser.yy"
                      {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("subnet6", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.SUBNET6);
}
#line 2253 "dhcp6_parser.cc"
    break;

  case 333:
#line 1248 "dhcp6_parser.yy"
                                                             {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2262 "dhcp6_parser.cc"
    break;

  case 338:
#line 1268 "dhcp6_parser.yy"
                        {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2272 "dhcp6_parser.cc"
    break;

  case 339:
#line 1272 "dhcp6_parser.yy"
                                {
    // Once we reached this place, the subnet parsing is now complete.
    // If we want to, we can implement default values here.
    // In particular we can do things like this:
    // if (!ctx.stack_.back()->get("interface")) {
    //     ctx.stack_.back()->set("interface", StringElement("loopback"));
    // }
    //
    // We can also stack up one level (Dhcp6) and copy over whatever
    // global parameters we want to:
    // if (!ctx.stack_.back()->get("renew-timer")) {
    //     ElementPtr renew = ctx_stack_[...].get("renew-timer");
    //     if (renew) {
    //         ctx.stack_.back()->set("renew-timer", renew);
    //     }
    // }

    // The subnet subnet6 parameter is required
    ctx.require("subnet", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2298 "dhcp6_parser.cc"
    break;

  case 340:
#line 1294 "dhcp6_parser.yy"
                            {
    // Parse the subnet6 list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2308 "dhcp6_parser.cc"
    break;

  case 341:
#line 1298 "dhcp6_parser.yy"
                                {
    // The subnet subnet6 parameter is required
    ctx.require("subnet", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2318 "dhcp6_parser.cc"
    break;

  case 381:
#line 1349 "dhcp6_parser.yy"
               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2326 "dhcp6_parser.cc"
    break;

  case 382:
#line 1351 "dhcp6_parser.yy"
               {
    ElementPtr subnet(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("subnet", subnet);
    ctx.leave();
}
#line 2336 "dhcp6_parser.cc"
    break;

  case 383:
#line 1357 "dhcp6_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2344 "dhcp6_parser.cc"
    break;

  case 384:
#line 1359 "dhcp6_parser.yy"
               {
    ElementPtr iface(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interface", iface);
    ctx.leave();
}
#line 2354 "dhcp6_parser.cc"
    break;

  case 385:
#line 1365 "dhcp6_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2362 "dhcp6_parser.cc"
    break;

  case 386:
#line 1367 "dhcp6_parser.yy"
               {
    ElementPtr iface(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("interface-id", iface);
    ctx.leave();
}
#line 2372 "dhcp6_parser.cc"
    break;

  case 387:
#line 1373 "dhcp6_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2380 "dhcp6_parser.cc"
    break;

  case 388:
#line 1375 "dhcp6_parser.yy"
               {
    ElementPtr cls(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-class", cls);
    ctx.leave();
}
#line 2390 "dhcp6_parser.cc"
    break;

  case 389:
#line 1381 "dhcp6_parser.yy"
                                               {
    ElementPtr c(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("require-client-classes", c);
    ctx.stack_.push_back(c);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2401 "dhcp6_parser.cc"
    break;

  case 390:
#line 1386 "dhcp6_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2410 "dhcp6_parser.cc"
    break;

  case 391:
#line 1391 "dhcp6_parser.yy"
                                   {
    ctx.enter(ctx.RESERVATION_MODE);
}
#line 2418 "dhcp6_parser.cc"
    break;

  case 392:
#line 1393 "dhcp6_parser.yy"
                {
    ctx.stack_.back()->set("reservation-mode", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 2427 "dhcp6_parser.cc"
    break;

  case 393:
#line 1398 "dhcp6_parser.yy"
                  { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("disabled", ctx.loc2pos(yystack_[0].location))); }
#line 2433 "dhcp6_parser.cc"
    break;

  case 394:
#line 1399 "dhcp6_parser.yy"
                     { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("out-of-pool", ctx.loc2pos(yystack_[0].location))); }
#line 2439 "dhcp6_parser.cc"
    break;

  case 395:
#line 1400 "dhcp6_parser.yy"
                { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("global", ctx.loc2pos(yystack_[0].location))); }
#line 2445 "dhcp6_parser.cc"
    break;

  case 396:
#line 1401 "dhcp6_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("all", ctx.loc2pos(yystack_[0].location))); }
#line 2451 "dhcp6_parser.cc"
    break;

  case 397:
#line 1404 "dhcp6_parser.yy"
                     {
    ElementPtr id(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("id", id);
}
#line 2460 "dhcp6_parser.cc"
    break;

  case 398:
#line 1409 "dhcp6_parser.yy"
                                         {
    ElementPtr rc(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("rapid-commit", rc);
}
#line 2469 "dhcp6_parser.cc"
    break;

  case 399:
#line 1416 "dhcp6_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("shared-networks", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.SHARED_NETWORK);
}
#line 2480 "dhcp6_parser.cc"
    break;

  case 400:
#line 1421 "dhcp6_parser.yy"
                                                                {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2489 "dhcp6_parser.cc"
    break;

  case 405:
#line 1436 "dhcp6_parser.yy"
                               {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2499 "dhcp6_parser.cc"
    break;

  case 406:
#line 1440 "dhcp6_parser.yy"
                                       {
    ctx.stack_.pop_back();
}
#line 2507 "dhcp6_parser.cc"
    break;

  case 442:
#line 1487 "dhcp6_parser.yy"
                            {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-def", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OPTION_DEF);
}
#line 2518 "dhcp6_parser.cc"
    break;

  case 443:
#line 1492 "dhcp6_parser.yy"
                                                                {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2527 "dhcp6_parser.cc"
    break;

  case 444:
#line 1500 "dhcp6_parser.yy"
                                    {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2536 "dhcp6_parser.cc"
    break;

  case 445:
#line 1503 "dhcp6_parser.yy"
                                 {
    // parsing completed
}
#line 2544 "dhcp6_parser.cc"
    break;

  case 450:
#line 1519 "dhcp6_parser.yy"
                                 {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2554 "dhcp6_parser.cc"
    break;

  case 451:
#line 1523 "dhcp6_parser.yy"
                                   {
    // The name, code and type option def parameters are required.
    ctx.require("name", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("code", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("type", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2566 "dhcp6_parser.cc"
    break;

  case 452:
#line 1534 "dhcp6_parser.yy"
                               {
    // Parse the option-def list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2576 "dhcp6_parser.cc"
    break;

  case 453:
#line 1538 "dhcp6_parser.yy"
                                   {
    // The name, code and type option def parameters are required.
    ctx.require("name", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("code", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("type", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2588 "dhcp6_parser.cc"
    break;

  case 469:
#line 1570 "dhcp6_parser.yy"
                         {
    ElementPtr code(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("code", code);
}
#line 2597 "dhcp6_parser.cc"
    break;

  case 471:
#line 1577 "dhcp6_parser.yy"
                      {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2605 "dhcp6_parser.cc"
    break;

  case 472:
#line 1579 "dhcp6_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("type", prf);
    ctx.leave();
}
#line 2615 "dhcp6_parser.cc"
    break;

  case 473:
#line 1585 "dhcp6_parser.yy"
                                      {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2623 "dhcp6_parser.cc"
    break;

  case 474:
#line 1587 "dhcp6_parser.yy"
               {
    ElementPtr rtypes(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("record-types", rtypes);
    ctx.leave();
}
#line 2633 "dhcp6_parser.cc"
    break;

  case 475:
#line 1593 "dhcp6_parser.yy"
             {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2641 "dhcp6_parser.cc"
    break;

  case 476:
#line 1595 "dhcp6_parser.yy"
               {
    ElementPtr space(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("space", space);
    ctx.leave();
}
#line 2651 "dhcp6_parser.cc"
    break;

  case 478:
#line 1603 "dhcp6_parser.yy"
                                    {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2659 "dhcp6_parser.cc"
    break;

  case 479:
#line 1605 "dhcp6_parser.yy"
               {
    ElementPtr encap(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("encapsulate", encap);
    ctx.leave();
}
#line 2669 "dhcp6_parser.cc"
    break;

  case 480:
#line 1611 "dhcp6_parser.yy"
                                      {
    ElementPtr array(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("array", array);
}
#line 2678 "dhcp6_parser.cc"
    break;

  case 481:
#line 1620 "dhcp6_parser.yy"
                              {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-data", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OPTION_DATA);
}
#line 2689 "dhcp6_parser.cc"
    break;

  case 482:
#line 1625 "dhcp6_parser.yy"
                                                                 {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2698 "dhcp6_parser.cc"
    break;

  case 487:
#line 1644 "dhcp6_parser.yy"
                                  {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2708 "dhcp6_parser.cc"
    break;

  case 488:
#line 1648 "dhcp6_parser.yy"
                                    {
    /// @todo: the code or name parameters are required.
    ctx.stack_.pop_back();
}
#line 2717 "dhcp6_parser.cc"
    break;

  case 489:
#line 1656 "dhcp6_parser.yy"
                                {
    // Parse the option-data list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2727 "dhcp6_parser.cc"
    break;

  case 490:
#line 1660 "dhcp6_parser.yy"
                                    {
    /// @todo: the code or name parameters are required.
    // parsing completed
}
#line 2736 "dhcp6_parser.cc"
    break;

  case 505:
#line 1693 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2744 "dhcp6_parser.cc"
    break;

  case 506:
#line 1695 "dhcp6_parser.yy"
               {
    ElementPtr data(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("data", data);
    ctx.leave();
}
#line 2754 "dhcp6_parser.cc"
    break;

  case 509:
#line 1705 "dhcp6_parser.yy"
                                                 {
    ElementPtr space(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("csv-format", space);
}
#line 2763 "dhcp6_parser.cc"
    break;

  case 510:
#line 1710 "dhcp6_parser.yy"
                                                   {
    ElementPtr persist(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("always-send", persist);
}
#line 2772 "dhcp6_parser.cc"
    break;

  case 511:
#line 1718 "dhcp6_parser.yy"
                  {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pools", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.POOLS);
}
#line 2783 "dhcp6_parser.cc"
    break;

  case 512:
#line 1723 "dhcp6_parser.yy"
                                                           {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2792 "dhcp6_parser.cc"
    break;

  case 517:
#line 1738 "dhcp6_parser.yy"
                                {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2802 "dhcp6_parser.cc"
    break;

  case 518:
#line 1742 "dhcp6_parser.yy"
                             {
    // The pool parameter is required.
    ctx.require("pool", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2812 "dhcp6_parser.cc"
    break;

  case 519:
#line 1748 "dhcp6_parser.yy"
                          {
    // Parse the pool list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2822 "dhcp6_parser.cc"
    break;

  case 520:
#line 1752 "dhcp6_parser.yy"
                             {
    // The pool parameter is required.
    ctx.require("pool", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2832 "dhcp6_parser.cc"
    break;

  case 532:
#line 1773 "dhcp6_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2840 "dhcp6_parser.cc"
    break;

  case 533:
#line 1775 "dhcp6_parser.yy"
               {
    ElementPtr pool(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pool", pool);
    ctx.leave();
}
#line 2850 "dhcp6_parser.cc"
    break;

  case 534:
#line 1781 "dhcp6_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2858 "dhcp6_parser.cc"
    break;

  case 535:
#line 1783 "dhcp6_parser.yy"
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
#line 2885 "dhcp6_parser.cc"
    break;

  case 536:
#line 1806 "dhcp6_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2893 "dhcp6_parser.cc"
    break;

  case 537:
#line 1808 "dhcp6_parser.yy"
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
#line 2922 "dhcp6_parser.cc"
    break;

  case 538:
#line 1836 "dhcp6_parser.yy"
                        {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pd-pools", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.PD_POOLS);
}
#line 2933 "dhcp6_parser.cc"
    break;

  case 539:
#line 1841 "dhcp6_parser.yy"
                                                              {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 2942 "dhcp6_parser.cc"
    break;

  case 544:
#line 1856 "dhcp6_parser.yy"
                              {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 2952 "dhcp6_parser.cc"
    break;

  case 545:
#line 1860 "dhcp6_parser.yy"
                                {
    // The prefix, prefix len and delegated len parameters are required.
    ctx.require("prefix", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("prefix-len", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("delegated-len", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 2964 "dhcp6_parser.cc"
    break;

  case 546:
#line 1868 "dhcp6_parser.yy"
                            {
    // Parse the pd-pool list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 2974 "dhcp6_parser.cc"
    break;

  case 547:
#line 1872 "dhcp6_parser.yy"
                                {
    // The prefix, prefix len and delegated len parameters are required.
    ctx.require("prefix", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("prefix-len", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.require("delegated-len", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 2986 "dhcp6_parser.cc"
    break;

  case 563:
#line 1899 "dhcp6_parser.yy"
                  {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 2994 "dhcp6_parser.cc"
    break;

  case 564:
#line 1901 "dhcp6_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("prefix", prf);
    ctx.leave();
}
#line 3004 "dhcp6_parser.cc"
    break;

  case 565:
#line 1907 "dhcp6_parser.yy"
                                        {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("prefix-len", prf);
}
#line 3013 "dhcp6_parser.cc"
    break;

  case 566:
#line 1912 "dhcp6_parser.yy"
                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3021 "dhcp6_parser.cc"
    break;

  case 567:
#line 1914 "dhcp6_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("excluded-prefix", prf);
    ctx.leave();
}
#line 3031 "dhcp6_parser.cc"
    break;

  case 568:
#line 1920 "dhcp6_parser.yy"
                                                       {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("excluded-prefix-len", prf);
}
#line 3040 "dhcp6_parser.cc"
    break;

  case 569:
#line 1925 "dhcp6_parser.yy"
                                              {
    ElementPtr deleg(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("delegated-len", deleg);
}
#line 3049 "dhcp6_parser.cc"
    break;

  case 570:
#line 1933 "dhcp6_parser.yy"
                           {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("reservations", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.RESERVATIONS);
}
#line 3060 "dhcp6_parser.cc"
    break;

  case 571:
#line 1938 "dhcp6_parser.yy"
                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3069 "dhcp6_parser.cc"
    break;

  case 576:
#line 1951 "dhcp6_parser.yy"
                            {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 3079 "dhcp6_parser.cc"
    break;

  case 577:
#line 1955 "dhcp6_parser.yy"
                                    {
    /// @todo: an identifier parameter is required.
    ctx.stack_.pop_back();
}
#line 3088 "dhcp6_parser.cc"
    break;

  case 578:
#line 1960 "dhcp6_parser.yy"
                                {
    // Parse the reservations list entry map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 3098 "dhcp6_parser.cc"
    break;

  case 579:
#line 1964 "dhcp6_parser.yy"
                                    {
    /// @todo: an identifier parameter is required.
    // parsing completed
}
#line 3107 "dhcp6_parser.cc"
    break;

  case 595:
#line 1991 "dhcp6_parser.yy"
                           {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ip-addresses", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3118 "dhcp6_parser.cc"
    break;

  case 596:
#line 1996 "dhcp6_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3127 "dhcp6_parser.cc"
    break;

  case 597:
#line 2001 "dhcp6_parser.yy"
                   {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("prefixes", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3138 "dhcp6_parser.cc"
    break;

  case 598:
#line 2006 "dhcp6_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3147 "dhcp6_parser.cc"
    break;

  case 599:
#line 2011 "dhcp6_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3155 "dhcp6_parser.cc"
    break;

  case 600:
#line 2013 "dhcp6_parser.yy"
               {
    ElementPtr d(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("duid", d);
    ctx.leave();
}
#line 3165 "dhcp6_parser.cc"
    break;

  case 601:
#line 2019 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3173 "dhcp6_parser.cc"
    break;

  case 602:
#line 2021 "dhcp6_parser.yy"
               {
    ElementPtr hw(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hw-address", hw);
    ctx.leave();
}
#line 3183 "dhcp6_parser.cc"
    break;

  case 603:
#line 2027 "dhcp6_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3191 "dhcp6_parser.cc"
    break;

  case 604:
#line 2029 "dhcp6_parser.yy"
               {
    ElementPtr host(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname", host);
    ctx.leave();
}
#line 3201 "dhcp6_parser.cc"
    break;

  case 605:
#line 2035 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3209 "dhcp6_parser.cc"
    break;

  case 606:
#line 2037 "dhcp6_parser.yy"
               {
    ElementPtr hw(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flex-id", hw);
    ctx.leave();
}
#line 3219 "dhcp6_parser.cc"
    break;

  case 607:
#line 2043 "dhcp6_parser.yy"
                                           {
    ElementPtr c(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-classes", c);
    ctx.stack_.push_back(c);
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3230 "dhcp6_parser.cc"
    break;

  case 608:
#line 2048 "dhcp6_parser.yy"
                     {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3239 "dhcp6_parser.cc"
    break;

  case 609:
#line 2056 "dhcp6_parser.yy"
             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("relay", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.RELAY);
}
#line 3250 "dhcp6_parser.cc"
    break;

  case 610:
#line 2061 "dhcp6_parser.yy"
                                                {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3259 "dhcp6_parser.cc"
    break;

  case 613:
#line 2070 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3267 "dhcp6_parser.cc"
    break;

  case 614:
#line 2072 "dhcp6_parser.yy"
               {
    ElementPtr addr(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ip-address", addr);
    ctx.leave();
}
#line 3277 "dhcp6_parser.cc"
    break;

  case 615:
#line 2081 "dhcp6_parser.yy"
                               {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("client-classes", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.CLIENT_CLASSES);
}
#line 3288 "dhcp6_parser.cc"
    break;

  case 616:
#line 2086 "dhcp6_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3297 "dhcp6_parser.cc"
    break;

  case 619:
#line 2095 "dhcp6_parser.yy"
                                   {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 3307 "dhcp6_parser.cc"
    break;

  case 620:
#line 2099 "dhcp6_parser.yy"
                                     {
    // The name client class parameter is required.
    ctx.require("name", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
}
#line 3317 "dhcp6_parser.cc"
    break;

  case 633:
#line 2124 "dhcp6_parser.yy"
                        {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3325 "dhcp6_parser.cc"
    break;

  case 634:
#line 2126 "dhcp6_parser.yy"
               {
    ElementPtr test(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("test", test);
    ctx.leave();
}
#line 3335 "dhcp6_parser.cc"
    break;

  case 635:
#line 2132 "dhcp6_parser.yy"
                                                 {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("only-if-required", b);
}
#line 3344 "dhcp6_parser.cc"
    break;

  case 636:
#line 2140 "dhcp6_parser.yy"
                     {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-id", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.SERVER_ID);
}
#line 3355 "dhcp6_parser.cc"
    break;

  case 637:
#line 2145 "dhcp6_parser.yy"
                                                       {
    // The type parameter is required.
    ctx.require("type", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3366 "dhcp6_parser.cc"
    break;

  case 649:
#line 2167 "dhcp6_parser.yy"
                     {
    ctx.enter(ctx.DUID_TYPE);
}
#line 3374 "dhcp6_parser.cc"
    break;

  case 650:
#line 2169 "dhcp6_parser.yy"
                  {
    ctx.stack_.back()->set("type", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3383 "dhcp6_parser.cc"
    break;

  case 651:
#line 2174 "dhcp6_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("LLT", ctx.loc2pos(yystack_[0].location))); }
#line 3389 "dhcp6_parser.cc"
    break;

  case 652:
#line 2175 "dhcp6_parser.yy"
              { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("EN", ctx.loc2pos(yystack_[0].location))); }
#line 3395 "dhcp6_parser.cc"
    break;

  case 653:
#line 2176 "dhcp6_parser.yy"
              { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("LL", ctx.loc2pos(yystack_[0].location))); }
#line 3401 "dhcp6_parser.cc"
    break;

  case 654:
#line 2179 "dhcp6_parser.yy"
                           {
    ElementPtr htype(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("htype", htype);
}
#line 3410 "dhcp6_parser.cc"
    break;

  case 655:
#line 2184 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3418 "dhcp6_parser.cc"
    break;

  case 656:
#line 2186 "dhcp6_parser.yy"
               {
    ElementPtr id(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("identifier", id);
    ctx.leave();
}
#line 3428 "dhcp6_parser.cc"
    break;

  case 657:
#line 2192 "dhcp6_parser.yy"
                         {
    ElementPtr time(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("time", time);
}
#line 3437 "dhcp6_parser.cc"
    break;

  case 658:
#line 2197 "dhcp6_parser.yy"
                                           {
    ElementPtr time(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enterprise-id", time);
}
#line 3446 "dhcp6_parser.cc"
    break;

  case 659:
#line 2204 "dhcp6_parser.yy"
                                         {
    ElementPtr time(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp4o6-port", time);
}
#line 3455 "dhcp6_parser.cc"
    break;

  case 660:
#line 2211 "dhcp6_parser.yy"
                               {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("control-socket", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKET);
}
#line 3466 "dhcp6_parser.cc"
    break;

  case 661:
#line 2216 "dhcp6_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3475 "dhcp6_parser.cc"
    break;

  case 677:
#line 2244 "dhcp6_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3483 "dhcp6_parser.cc"
    break;

  case 678:
#line 2246 "dhcp6_parser.yy"
               {
    ElementPtr stype(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-type", stype);
    ctx.leave();
}
#line 3493 "dhcp6_parser.cc"
    break;

  case 679:
#line 2252 "dhcp6_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3501 "dhcp6_parser.cc"
    break;

  case 680:
#line 2254 "dhcp6_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-name", name);
    ctx.leave();
}
#line 3511 "dhcp6_parser.cc"
    break;

  case 681:
#line 2260 "dhcp6_parser.yy"
                                                                                                             {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("on-boot-request-configuration-from-kea-netconf", b);
}
#line 3520 "dhcp6_parser.cc"
    break;

  case 682:
#line 2267 "dhcp6_parser.yy"
                                       {
    ElementPtr qc(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp-queue-control", qc);
    ctx.stack_.push_back(qc);
    ctx.enter(ctx.DHCP_QUEUE_CONTROL);
}
#line 3531 "dhcp6_parser.cc"
    break;

  case 683:
#line 2272 "dhcp6_parser.yy"
                                                           {
    // The enable queue parameter is required.
    ctx.require("enable-queue", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3542 "dhcp6_parser.cc"
    break;

  case 692:
#line 2291 "dhcp6_parser.yy"
                                         {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enable-queue", b);
}
#line 3551 "dhcp6_parser.cc"
    break;

  case 693:
#line 2296 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3559 "dhcp6_parser.cc"
    break;

  case 694:
#line 2298 "dhcp6_parser.yy"
               {
    ElementPtr qt(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("queue-type", qt);
    ctx.leave();
}
#line 3569 "dhcp6_parser.cc"
    break;

  case 695:
#line 2304 "dhcp6_parser.yy"
                                 {
    ElementPtr c(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("capacity", c);
}
#line 3578 "dhcp6_parser.cc"
    break;

  case 696:
#line 2309 "dhcp6_parser.yy"
                            {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3586 "dhcp6_parser.cc"
    break;

  case 697:
#line 2311 "dhcp6_parser.yy"
              {
    ctx.stack_.back()->set(yystack_[3].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3595 "dhcp6_parser.cc"
    break;

  case 698:
#line 2318 "dhcp6_parser.yy"
                     {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp-ddns", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.DHCP_DDNS);
}
#line 3606 "dhcp6_parser.cc"
    break;

  case 699:
#line 2323 "dhcp6_parser.yy"
                                                       {
    // The enable updates DHCP DDNS parameter is required.
    ctx.require("enable-updates", ctx.loc2pos(yystack_[2].location), ctx.loc2pos(yystack_[0].location));
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3617 "dhcp6_parser.cc"
    break;

  case 700:
#line 2330 "dhcp6_parser.yy"
                              {
    // Parse the dhcp-ddns map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 3627 "dhcp6_parser.cc"
    break;

  case 701:
#line 2334 "dhcp6_parser.yy"
                                  {
    // The enable updates DHCP DDNS parameter is required.
    ctx.require("enable-updates", ctx.loc2pos(yystack_[3].location), ctx.loc2pos(yystack_[0].location));
    // parsing completed
}
#line 3637 "dhcp6_parser.cc"
    break;

  case 722:
#line 2364 "dhcp6_parser.yy"
                                             {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("enable-updates", b);
}
#line 3646 "dhcp6_parser.cc"
    break;

  case 723:
#line 2369 "dhcp6_parser.yy"
                                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3654 "dhcp6_parser.cc"
    break;

  case 724:
#line 2371 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("qualifying-suffix", s);
    ctx.leave();
}
#line 3664 "dhcp6_parser.cc"
    break;

  case 725:
#line 2377 "dhcp6_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3672 "dhcp6_parser.cc"
    break;

  case 726:
#line 2379 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-ip", s);
    ctx.leave();
}
#line 3682 "dhcp6_parser.cc"
    break;

  case 727:
#line 2385 "dhcp6_parser.yy"
                                       {
    ElementPtr i(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("server-port", i);
}
#line 3691 "dhcp6_parser.cc"
    break;

  case 728:
#line 2390 "dhcp6_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3699 "dhcp6_parser.cc"
    break;

  case 729:
#line 2392 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("sender-ip", s);
    ctx.leave();
}
#line 3709 "dhcp6_parser.cc"
    break;

  case 730:
#line 2398 "dhcp6_parser.yy"
                                       {
    ElementPtr i(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("sender-port", i);
}
#line 3718 "dhcp6_parser.cc"
    break;

  case 731:
#line 2403 "dhcp6_parser.yy"
                                             {
    ElementPtr i(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-queue-size", i);
}
#line 3727 "dhcp6_parser.cc"
    break;

  case 732:
#line 2408 "dhcp6_parser.yy"
                           {
    ctx.enter(ctx.NCR_PROTOCOL);
}
#line 3735 "dhcp6_parser.cc"
    break;

  case 733:
#line 2410 "dhcp6_parser.yy"
                           {
    ctx.stack_.back()->set("ncr-protocol", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3744 "dhcp6_parser.cc"
    break;

  case 734:
#line 2416 "dhcp6_parser.yy"
        { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("UDP", ctx.loc2pos(yystack_[0].location))); }
#line 3750 "dhcp6_parser.cc"
    break;

  case 735:
#line 2417 "dhcp6_parser.yy"
        { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("TCP", ctx.loc2pos(yystack_[0].location))); }
#line 3756 "dhcp6_parser.cc"
    break;

  case 736:
#line 2420 "dhcp6_parser.yy"
                       {
    ctx.enter(ctx.NCR_FORMAT);
}
#line 3764 "dhcp6_parser.cc"
    break;

  case 737:
#line 2422 "dhcp6_parser.yy"
             {
    ElementPtr json(new StringElement("JSON", ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ncr-format", json);
    ctx.leave();
}
#line 3774 "dhcp6_parser.cc"
    break;

  case 738:
#line 2429 "dhcp6_parser.yy"
                                                         {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("override-no-update", b);
}
#line 3783 "dhcp6_parser.cc"
    break;

  case 739:
#line 2435 "dhcp6_parser.yy"
                                                                 {
    ElementPtr b(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("override-client-update", b);
}
#line 3792 "dhcp6_parser.cc"
    break;

  case 740:
#line 2441 "dhcp6_parser.yy"
                                             {
    ctx.enter(ctx.REPLACE_CLIENT_NAME);
}
#line 3800 "dhcp6_parser.cc"
    break;

  case 741:
#line 2443 "dhcp6_parser.yy"
                                       {
    ctx.stack_.back()->set("replace-client-name", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 3809 "dhcp6_parser.cc"
    break;

  case 742:
#line 2449 "dhcp6_parser.yy"
                                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3817 "dhcp6_parser.cc"
    break;

  case 743:
#line 2451 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("generated-prefix", s);
    ctx.leave();
}
#line 3827 "dhcp6_parser.cc"
    break;

  case 744:
#line 2458 "dhcp6_parser.yy"
                                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3835 "dhcp6_parser.cc"
    break;

  case 745:
#line 2460 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-set", s);
    ctx.leave();
}
#line 3845 "dhcp6_parser.cc"
    break;

  case 746:
#line 2467 "dhcp6_parser.yy"
                                                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3853 "dhcp6_parser.cc"
    break;

  case 747:
#line 2469 "dhcp6_parser.yy"
               {
    ElementPtr s(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hostname-char-replacement", s);
    ctx.leave();
}
#line 3863 "dhcp6_parser.cc"
    break;

  case 748:
#line 2478 "dhcp6_parser.yy"
                               {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-control", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.CONFIG_CONTROL);
}
#line 3874 "dhcp6_parser.cc"
    break;

  case 749:
#line 2483 "dhcp6_parser.yy"
                                                            {
    // No config control params are required
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3884 "dhcp6_parser.cc"
    break;

  case 750:
#line 2489 "dhcp6_parser.yy"
                                   {
    // Parse the config-control map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 3894 "dhcp6_parser.cc"
    break;

  case 751:
#line 2493 "dhcp6_parser.yy"
                                       {
    // No config_control params are required
    // parsing completed
}
#line 3903 "dhcp6_parser.cc"
    break;

  case 756:
#line 2508 "dhcp6_parser.yy"
                                   {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-databases", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.CONFIG_DATABASE);
}
#line 3914 "dhcp6_parser.cc"
    break;

  case 757:
#line 2513 "dhcp6_parser.yy"
                                                      {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3923 "dhcp6_parser.cc"
    break;

  case 758:
#line 2518 "dhcp6_parser.yy"
                                                             {
    ElementPtr value(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-fetch-wait-time", value);
}
#line 3932 "dhcp6_parser.cc"
    break;

  case 759:
#line 2525 "dhcp6_parser.yy"
                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("loggers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.LOGGERS);
}
#line 3943 "dhcp6_parser.cc"
    break;

  case 760:
#line 2530 "dhcp6_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 3952 "dhcp6_parser.cc"
    break;

  case 763:
#line 2542 "dhcp6_parser.yy"
                             {
    ElementPtr l(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(l);
    ctx.stack_.push_back(l);
}
#line 3962 "dhcp6_parser.cc"
    break;

  case 764:
#line 2546 "dhcp6_parser.yy"
                               {
    ctx.stack_.pop_back();
}
#line 3970 "dhcp6_parser.cc"
    break;

  case 774:
#line 2563 "dhcp6_parser.yy"
                                     {
    ElementPtr dl(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("debuglevel", dl);
}
#line 3979 "dhcp6_parser.cc"
    break;

  case 775:
#line 2568 "dhcp6_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 3987 "dhcp6_parser.cc"
    break;

  case 776:
#line 2570 "dhcp6_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("severity", sev);
    ctx.leave();
}
#line 3997 "dhcp6_parser.cc"
    break;

  case 777:
#line 2576 "dhcp6_parser.yy"
                                    {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output_options", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OUTPUT_OPTIONS);
}
#line 4008 "dhcp6_parser.cc"
    break;

  case 778:
#line 2581 "dhcp6_parser.yy"
                                                                    {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4017 "dhcp6_parser.cc"
    break;

  case 781:
#line 2590 "dhcp6_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 4027 "dhcp6_parser.cc"
    break;

  case 782:
#line 2594 "dhcp6_parser.yy"
                                    {
    ctx.stack_.pop_back();
}
#line 4035 "dhcp6_parser.cc"
    break;

  case 790:
#line 2609 "dhcp6_parser.yy"
               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4043 "dhcp6_parser.cc"
    break;

  case 791:
#line 2611 "dhcp6_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output", sev);
    ctx.leave();
}
#line 4053 "dhcp6_parser.cc"
    break;

  case 792:
#line 2617 "dhcp6_parser.yy"
                           {
    ElementPtr flush(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flush", flush);
}
#line 4062 "dhcp6_parser.cc"
    break;

  case 793:
#line 2622 "dhcp6_parser.yy"
                               {
    ElementPtr maxsize(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxsize", maxsize);
}
#line 4071 "dhcp6_parser.cc"
    break;

  case 794:
#line 2627 "dhcp6_parser.yy"
                             {
    ElementPtr maxver(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxver", maxver);
}
#line 4080 "dhcp6_parser.cc"
    break;

  case 795:
#line 2632 "dhcp6_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4088 "dhcp6_parser.cc"
    break;

  case 796:
#line 2634 "dhcp6_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pattern", sev);
    ctx.leave();
}
#line 4098 "dhcp6_parser.cc"
    break;

  case 797:
#line 2642 "dhcp6_parser.yy"
                                 {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4106 "dhcp6_parser.cc"
    break;

  case 798:
#line 2644 "dhcp6_parser.yy"
               {
    ElementPtr allocation_type(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("allocation-type", allocation_type);
    ctx.leave();
}
#line 4116 "dhcp6_parser.cc"
    break;

  case 799:
#line 2650 "dhcp6_parser.yy"
                                 {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("config-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.CONFIG_DATABASE);
}
#line 4127 "dhcp6_parser.cc"
    break;

  case 800:
#line 2655 "dhcp6_parser.yy"
                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4136 "dhcp6_parser.cc"
    break;

  case 801:
#line 2660 "dhcp6_parser.yy"
                                       {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4144 "dhcp6_parser.cc"
    break;

  case 802:
#line 2662 "dhcp6_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("configuration-type", prf);
    ctx.leave();
}
#line 4154 "dhcp6_parser.cc"
    break;

  case 803:
#line 2668 "dhcp6_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4162 "dhcp6_parser.cc"
    break;

  case 804:
#line 2670 "dhcp6_parser.yy"
               {
    ElementPtr prf(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("instance-id", prf);
    ctx.leave();
}
#line 4172 "dhcp6_parser.cc"
    break;

  case 805:
#line 2677 "dhcp6_parser.yy"
                                                               {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4180 "dhcp6_parser.cc"
    break;

  case 806:
#line 2679 "dhcp6_parser.yy"
              {
    ctx.stack_.back()->set("lawful-interception-parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 4189 "dhcp6_parser.cc"
    break;

  case 807:
#line 2684 "dhcp6_parser.yy"
                                 {
    ElementPtr i(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("master-database", i);
    ctx.stack_.push_back(i);
    ctx.enter(ctx.MASTER_DATABASE);
}
#line 4200 "dhcp6_parser.cc"
    break;

  case 808:
#line 2689 "dhcp6_parser.yy"
                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4209 "dhcp6_parser.cc"
    break;

  case 809:
#line 2694 "dhcp6_parser.yy"
                                                               {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-address-utilization", n);
}
#line 4218 "dhcp6_parser.cc"
    break;

  case 810:
#line 2699 "dhcp6_parser.yy"
                                                                 {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-pd-space-utilization", n);
}
#line 4227 "dhcp6_parser.cc"
    break;

  case 811:
#line 2704 "dhcp6_parser.yy"
                                                       {
    ElementPtr n(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("max-statement-tries", n);
}
#line 4236 "dhcp6_parser.cc"
    break;

  case 812:
#line 2709 "dhcp6_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("notifications", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKET);
}
#line 4247 "dhcp6_parser.cc"
    break;

  case 813:
#line 2714 "dhcp6_parser.yy"
                                                                          {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 4256 "dhcp6_parser.cc"
    break;

  case 814:
#line 2719 "dhcp6_parser.yy"
                                                            {
    ElementPtr l(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-set-description", l);
}
#line 4265 "dhcp6_parser.cc"
    break;

  case 815:
#line 2724 "dhcp6_parser.yy"
                                           {
    ElementPtr l(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("option-set-id", l);
}
#line 4274 "dhcp6_parser.cc"
    break;

  case 816:
#line 2729 "dhcp6_parser.yy"
                                                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4282 "dhcp6_parser.cc"
    break;

  case 817:
#line 2731 "dhcp6_parser.yy"
              {
    ctx.stack_.back()->set("policy-engine-parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 4291 "dhcp6_parser.cc"
    break;

  case 818:
#line 2736 "dhcp6_parser.yy"
                                                         {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("privacy-history-size", prf);
}
#line 4300 "dhcp6_parser.cc"
    break;

  case 819:
#line 2741 "dhcp6_parser.yy"
                                                             {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("privacy-valid-lifetime", prf);
}
#line 4309 "dhcp6_parser.cc"
    break;

  case 820:
#line 2746 "dhcp6_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4317 "dhcp6_parser.cc"
    break;

  case 821:
#line 2748 "dhcp6_parser.yy"
               {
    ElementPtr protocol(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("protocol", protocol);
    ctx.leave();
}
#line 4327 "dhcp6_parser.cc"
    break;

  case 822:
#line 2754 "dhcp6_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORD);
}
#line 4335 "dhcp6_parser.cc"
    break;

  case 823:
#line 2756 "dhcp6_parser.yy"
               {
    ElementPtr cp(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("ssl-cert", cp);
    ctx.leave();
}
#line 4345 "dhcp6_parser.cc"
    break;


#line 4349 "dhcp6_parser.cc"

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
  Dhcp6Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  Dhcp6Parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
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


  const short Dhcp6Parser::yypact_ninf_ = -1019;

  const signed char Dhcp6Parser::yytable_ninf_ = -1;

  const short
  Dhcp6Parser::yypact_[] =
  {
     536, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019,    67,    33,    85,    92,    94,
     130,   140,   148,   208,   212,   224,   235,   236,   237,   243,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,    33,  -174,
      59,   123,    45,   538,   326,   194,   312,    -7,    77,   131,
     118,   521,    36, -1019,    78,   163,   269,   274,   281, -1019,
      56, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,   296,
     300,   306,   310,   316,   333,   338,   353,   374,   375,   377,
     385, -1019,   387,   404,   414,   416,   417, -1019, -1019, -1019,
     425, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019,   432, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
     433,   442,   443,   450,   456, -1019,    60, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
     458, -1019,    72, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019,   459,   460, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
      75, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,   462,
   -1019, -1019, -1019, -1019, -1019,   110, -1019, -1019, -1019, -1019,
   -1019, -1019,   464, -1019,   465,   466,   467, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019,   121, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019,   394,   390, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019,   403, -1019, -1019,   468, -1019, -1019, -1019,
     471, -1019, -1019,   470,   481, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,   485,   488,
   -1019, -1019, -1019, -1019,   477,   491, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019,   125, -1019, -1019, -1019, -1019, -1019, -1019,   492, -1019,
   -1019,   495, -1019,   496,   498, -1019, -1019,   499,   500, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019,   203, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019,   502,   204, -1019, -1019, -1019, -1019,
      33,    33, -1019,   294,   504,    59, -1019,   505,   506,   508,
     511,   512,   513,   308,   313,   319,   321,   323,   330,   336,
     339,   304,   341,   342,   340,   524,   354,   355,   322,   345,
     346,   560,   561,   563,   356,   569,   571,   572,   573,   574,
     576,   577,   589,   590,   591,   592,   600,   601,   605,   606,
     607,   399,   608,   610,   611,   613,   615,   616,   617,   618,
     619,   628,   629,   630,   422,   423,   424,   426, -1019,   123,
   -1019,   632,   429,    45, -1019,   635,   637,   638,   640,   644,
     436,   435,   649,   651,   652,   653,   538, -1019,   654,   445,
     326, -1019,   656,   447,   658,   449,   451,   452,   194, -1019,
     660,   663,   665,   666,   680,   684,   686, -1019,   312, -1019,
     689,   690,   482,   691,   694,   695,   484, -1019,    77,   706,
     501,   507, -1019,   131,   707,   708,   710,   711,   712,   -64,
   -1019,   509,   714,   716,   510,   718,   515,   516,   722,   727,
     517,   533,   728,   746,   748,   749,   521, -1019,   755,   546,
      36, -1019, -1019, -1019,   757,   720, -1019,   549,   756,   758,
     771,   772,   759, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019,   567, -1019, -1019, -1019, -1019,
   -1019,   180,   568,   580, -1019,   785,   789,   790,   791,   583,
     347,   792,   794,   795,   796,   797,   798,   800,   801,   804,
     807, -1019,   810,   811,   812,   813,   594,   595,   816,   815,
     612,   614,   817,   819, -1019, -1019, -1019, -1019, -1019,   818,
   -1019, -1019,   824,   825,   620,   621,   622, -1019, -1019,   818,
     623,   830,   625, -1019,   627, -1019, -1019,   631, -1019,   633,
   -1019, -1019, -1019, -1019,   818,   818,   818,   634,   636,   639,
     641, -1019,   642,   643, -1019,   645,   646,   647, -1019, -1019,
     648, -1019, -1019, -1019,   650,   655,   791,    33,    33, -1019,
   -1019,   657,   659, -1019,   661, -1019, -1019,  -118,   667, -1019,
   -1019,   180,   662,   664,   668, -1019,   836, -1019, -1019,    33,
     123, -1019,    36,    45,   334,   334,   835, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019,   838,   841,   843, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019,   844,   -77,    33,
     343,   699,   846,   850,   855,   260,    23,    58,     7,   177,
     521, -1019, -1019,   857,   334, -1019, -1019,   334,    17, -1019,
   -1019,   858,   859, -1019, -1019, -1019, -1019, -1019,   -59, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019,   835, -1019,   232,   248,   266, -1019, -1019, -1019,
   -1019,   839,   863,   865,   867,   869, -1019,   872,   874, -1019,
   -1019, -1019,   875,   876,   878,   879,   880, -1019, -1019, -1019,
     288, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019,   289, -1019,   881,   882, -1019,
   -1019,   883,   885, -1019, -1019,   884,   888, -1019, -1019,   886,
     892, -1019, -1019,   890,   894, -1019, -1019, -1019,   108, -1019,
   -1019, -1019,   893, -1019, -1019, -1019,   159, -1019, -1019, -1019,
   -1019,   314, -1019, -1019, -1019,   264, -1019, -1019,   895,   897,
   -1019, -1019,   896,   900, -1019,   901,   902,   903,   904,   905,
     906,   318, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019,   907,   908,   909, -1019, -1019, -1019, -1019,   324, -1019,
   -1019, -1019, -1019, -1019, -1019,   910,   911,   912, -1019,   325,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019,   327, -1019, -1019, -1019,   913, -1019,   914, -1019, -1019,
   -1019,   331, -1019, -1019, -1019, -1019, -1019,   335, -1019,   295,
   -1019,   348,   350,   915, -1019, -1019, -1019,   351, -1019, -1019,
   -1019, -1019,   673, -1019,   898,   917, -1019, -1019,   916,   918,
   -1019, -1019, -1019,   919, -1019,   920, -1019, -1019, -1019,   921,
     924,   925,   926,   709,   682,   717,   719,   724,   928,   725,
     726,   929,   930,   933,   729,   730,   731,   732,   734,   937,
     938,   334, -1019, -1019,   334, -1019,   835,   538, -1019,   838,
      77, -1019,   841,   131, -1019,   843,   723, -1019,   844,   -77,
   -1019, -1019,   343, -1019,   941,   699, -1019,   127,   846, -1019,
     312, -1019,   850,   118, -1019,   855,   735,   736,   737,   738,
     739,   740,   260, -1019,   951,   952,   743,   744,   745,    23,
   -1019,   747,   750,   751,    58, -1019,   956,   957,     7, -1019,
     752,   958,   753,   966,   177, -1019, -1019,    53,   857, -1019,
   -1019, -1019,   765,    17, -1019, -1019,   965,   971,   326, -1019,
     858,   194, -1019,   859,   979, -1019, -1019,   428,   774,   780,
     781, -1019, -1019, -1019, -1019, -1019,   783, -1019, -1019,   786,
     802,   806, -1019, -1019, -1019, -1019, -1019,   809,   821, -1019,
     389, -1019,   392, -1019,   976, -1019,   990, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019,   398, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019,   823, -1019, -1019,   999,
   -1019, -1019, -1019, -1019, -1019,  1006,  1004, -1019, -1019, -1019,
   -1019, -1019,  1015, -1019,   401, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019,    62,   833, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019,   834,   849, -1019, -1019,   852, -1019,    33,
   -1019, -1019,  1024, -1019, -1019, -1019, -1019, -1019,   407, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019,   870,   411, -1019,
     427, -1019,   891, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019,   723, -1019, -1019,  1027,   827, -1019,   127, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,  1045,
     854,  1048,    53, -1019, -1019, -1019, -1019, -1019, -1019,   899,
   -1019, -1019,  1052, -1019,   922, -1019, -1019,  1056, -1019, -1019,
     305, -1019,   109,  1056, -1019, -1019,  1068,  1090,  1094, -1019,
     430, -1019, -1019, -1019, -1019, -1019, -1019, -1019,  1099,   889,
     923,   927,  1103,   109, -1019,   931, -1019, -1019, -1019,   932,
   -1019, -1019, -1019
  };

  const short
  Dhcp6Parser::yydefact_[] =
  {
       0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,    45,    38,    34,    33,    30,    31,    32,    37,     3,
      35,    36,    58,     5,    65,     7,   170,     9,   340,    11,
     519,    13,   546,    15,   578,    17,   444,    19,   452,    21,
     489,    23,   300,    25,   700,    27,   750,    29,    47,    41,
       0,     0,     0,     0,     0,     0,   580,     0,   454,   491,
       0,     0,     0,    49,     0,    48,     0,     0,    42,    63,
       0,    60,    62,   131,   748,   168,   182,   184,   186,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   166,     0,     0,     0,     0,     0,   148,   155,   157,
       0,   332,   442,   481,   534,   536,   391,   399,   260,   277,
     268,   253,   615,   570,   292,   316,   636,     0,   279,   660,
     682,   698,   159,   161,   759,   799,   801,   803,   807,   812,
       0,     0,     0,     0,     0,   130,     0,    67,    69,    70,
      71,    72,    73,    74,    75,    76,    77,   105,   106,   107,
      78,   111,   112,   113,   114,   115,   116,   109,   110,   117,
     118,   119,   103,    81,    82,    83,    84,   100,    85,    87,
      86,   120,    91,    92,    79,   104,    80,    89,    90,    98,
      99,   101,    88,    93,    94,    95,    96,    97,   102,   108,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   179,
       0,   178,     0,   172,   174,   175,   176,   177,   511,   538,
     381,   383,   385,     0,     0,   389,   387,   609,   797,   380,
     344,   345,   346,   347,   348,   349,   350,   351,   367,   368,
     369,   372,   373,   374,   375,   376,   377,   370,   371,   378,
       0,   342,   355,   356,   357,   360,   361,   363,   358,   359,
     352,   353,   365,   366,   354,   362,   364,   379,   532,     0,
     531,   525,   526,   529,   524,     0,   521,   523,   527,   528,
     530,   563,     0,   566,     0,     0,     0,   562,   554,   555,
     560,   553,   558,   559,     0,   548,   550,   551,   556,   557,
     552,   561,   607,   595,   597,   599,   601,   603,   605,   594,
     591,   592,   593,     0,   581,   582,   586,   587,   584,   588,
     589,   590,   585,     0,   471,   233,     0,   475,   473,   478,
       0,   467,   468,     0,   455,   456,   458,   470,   459,   460,
     461,   477,   462,   463,   464,   465,   466,   505,     0,     0,
     503,   504,   507,   508,     0,   492,   493,   495,   496,   497,
     498,   499,   500,   501,   502,   310,   312,   314,   805,   816,
     304,     0,   302,   305,   306,   307,   308,   309,     0,   723,
     725,     0,   728,     0,     0,   732,   736,     0,     0,   740,
     742,   744,   746,   721,   719,   720,     0,   702,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   756,     0,     0,   752,   754,   755,    46,
       0,     0,    39,     0,     0,     0,    59,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,     0,
      66,     0,     0,     0,   171,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,     0,     0,
       0,   520,     0,     0,     0,     0,     0,     0,     0,   547,
       0,     0,     0,     0,     0,     0,     0,   579,     0,   445,
       0,     0,     0,     0,     0,     0,     0,   453,     0,     0,
       0,     0,   490,     0,     0,     0,     0,     0,     0,     0,
     301,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   701,     0,     0,
       0,   751,    50,    43,     0,     0,    61,     0,     0,     0,
       0,     0,     0,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,     0,   164,   165,   145,   146,
     147,     0,     0,     0,   163,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   659,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   814,   815,   818,   819,    68,     0,
     181,   173,     0,     0,     0,     0,     0,   397,   398,     0,
       0,     0,     0,   343,     0,   809,   522,     0,   565,     0,
     568,   569,   810,   549,     0,     0,     0,     0,     0,     0,
       0,   583,     0,     0,   469,     0,     0,     0,   480,   457,
       0,   509,   510,   494,     0,     0,     0,     0,     0,   303,
     722,     0,     0,   727,     0,   730,   731,     0,     0,   738,
     739,     0,     0,     0,     0,   703,     0,   758,   753,     0,
       0,   132,     0,     0,     0,     0,   188,   167,   150,   151,
     152,   153,   154,   149,   156,   158,   334,   446,   483,    40,
     535,   537,   393,   394,   395,   396,   392,   401,     0,    47,
       0,     0,     0,   572,   294,     0,     0,     0,     0,     0,
       0,   160,   162,     0,     0,   802,   804,     0,     0,    51,
     180,   513,   540,   382,   384,   386,   390,   388,     0,   798,
     533,   564,   567,   608,   596,   598,   600,   602,   604,   606,
     472,   234,   476,   474,   479,   506,   311,   313,   315,   806,
     817,   724,   726,   729,   734,   735,   733,   737,   741,   743,
     745,   747,   188,    44,     0,     0,     0,   220,   226,   228,
     230,     0,     0,     0,     0,     0,   244,     0,     0,   247,
     249,   251,     0,     0,     0,     0,     0,   820,   822,   219,
       0,   194,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   208,   215,   209,   210,   211,   206,   207,   212,
     213,   214,   216,   217,   218,     0,   192,     0,   189,   190,
     338,     0,   335,   336,   450,     0,   447,   448,   487,     0,
     484,   485,   405,     0,   402,   403,   266,   267,     0,   262,
     264,   265,     0,   275,   276,   272,     0,   270,   273,   274,
     258,     0,   255,   257,   619,     0,   617,   576,     0,   573,
     574,   298,     0,   295,   296,     0,     0,     0,     0,     0,
       0,     0,   318,   320,   321,   322,   323,   324,   325,   649,
     655,     0,     0,     0,   648,   645,   646,   647,     0,   638,
     640,   643,   641,   642,   644,     0,     0,     0,   288,     0,
     281,   283,   284,   285,   286,   287,   677,   679,   668,   666,
     667,     0,   662,   664,   665,     0,   693,     0,   696,   689,
     690,     0,   684,   686,   687,   688,   691,     0,   763,     0,
     761,     0,     0,     0,   676,   673,   674,     0,   669,   671,
     672,   675,    53,   517,     0,   514,   515,   544,     0,   541,
     542,   613,   612,     0,   611,     0,    64,   749,   169,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,   185,     0,   187,     0,     0,   333,     0,
     454,   443,     0,   491,   482,     0,     0,   400,     0,     0,
     261,   278,     0,   269,     0,     0,   254,   621,     0,   616,
     580,   571,     0,     0,   293,     0,     0,     0,     0,     0,
       0,     0,     0,   317,     0,     0,     0,     0,     0,     0,
     637,     0,     0,     0,     0,   280,     0,     0,     0,   661,
       0,     0,     0,     0,     0,   683,   699,     0,     0,   760,
     800,   808,     0,     0,   813,    55,     0,    54,     0,   512,
       0,     0,   539,     0,     0,   610,   757,     0,     0,     0,
       0,   232,   235,   236,   237,   238,     0,   246,   239,     0,
       0,     0,   241,   242,   243,   240,   811,     0,     0,   195,
       0,   191,     0,   337,     0,   449,     0,   486,   441,   420,
     421,   422,   424,   425,   426,   413,   414,   429,   430,   431,
     434,   435,   436,   437,   438,   439,   432,   433,   440,   409,
     410,   411,   412,   418,   419,   417,   423,     0,   407,   415,
     427,   428,   416,   404,   263,   271,     0,   256,   633,     0,
     631,   632,   628,   629,   630,     0,   622,   623,   625,   626,
     627,   618,     0,   575,     0,   297,   326,   327,   328,   329,
     330,   331,   319,     0,     0,   654,   657,   658,   639,   289,
     290,   291,   282,     0,     0,   663,   692,     0,   695,     0,
     685,   777,     0,   775,   773,   767,   771,   772,     0,   765,
     769,   770,   768,   762,   681,   670,    52,     0,     0,   516,
       0,   543,     0,   222,   223,   224,   225,   221,   227,   229,
     231,   245,   248,   250,   252,   821,   823,   193,   339,   451,
     488,     0,   406,   259,     0,     0,   620,     0,   577,   299,
     651,   652,   653,   650,   656,   678,   680,   694,   697,     0,
       0,     0,     0,   764,    56,   518,   545,   614,   408,     0,
     635,   624,     0,   774,     0,   766,   634,     0,   776,   781,
       0,   779,     0,     0,   778,   790,     0,     0,     0,   795,
       0,   783,   785,   786,   787,   788,   789,   780,     0,     0,
       0,     0,     0,     0,   782,     0,   792,   793,   794,     0,
     784,   791,   796
  };

  const short
  Dhcp6Parser::yypgoto_[] =
  {
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019,     9, -1019,  -577, -1019,
     228, -1019, -1019, -1019, -1019,   359, -1019,  -568, -1019, -1019,
   -1019,   -71, -1019, -1019, -1019,   675, -1019, -1019, -1019, -1019,
     391,   624, -1019, -1019,   -42,   -40,   -37,   -36,   -26,   -25,
     -21,   -19,     8,    13,    24, -1019,    27,    29,    32,    39,
   -1019,   402,    42, -1019,    43, -1019,    44, -1019,    47, -1019,
      50, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,   393,
     669, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
     302, -1019,    79, -1019,  -682,    86, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019,   -67, -1019,  -734, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,    63,
   -1019, -1019, -1019, -1019, -1019,    70,  -723, -1019, -1019, -1019,
   -1019,    68, -1019, -1019, -1019, -1019, -1019, -1019, -1019,    37,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019,    61, -1019, -1019,
   -1019,    64,   565, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019,    66, -1019, -1019, -1019, -1019, -1019, -1019, -1018,
   -1019, -1019, -1019,    89, -1019, -1019, -1019,    93,   626, -1019,
   -1019, -1017, -1019, -1016, -1019,    21, -1019,    51, -1019,    54,
   -1019, -1019,    -9, -1014, -1019, -1019, -1019, -1019,    83, -1019,
   -1019,  -138,  1057, -1019, -1019, -1019, -1019, -1019,    97, -1019,
   -1019, -1019,    96, -1019,   598, -1019,   -66, -1019, -1019, -1019,
   -1019, -1019,   -65, -1019, -1019, -1019, -1019, -1019,    14, -1019,
   -1019, -1019,    95, -1019, -1019, -1019,   100, -1019,   596, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
      38, -1019, -1019, -1019,    46,   670, -1019, -1019,   -55, -1019,
      -2, -1019, -1019, -1019, -1019, -1019,    34, -1019, -1019, -1019,
      40,   671, -1019, -1019, -1019, -1019, -1019, -1019, -1019,   -57,
   -1019, -1019, -1019,    88, -1019, -1019, -1019,    98, -1019,   674,
     378, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1012, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019,    99, -1019, -1019, -1019,  -125, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019,    74, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019,    73, -1019,    52,
    -762, -1019,  -753, -1019, -1019, -1019, -1019, -1019,    69, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,   400,
     588, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
     444,   585, -1019, -1019, -1019, -1019, -1019, -1019,    71, -1019,
   -1019,  -124, -1019, -1019, -1019, -1019, -1019, -1019,  -146, -1019,
   -1019,  -163, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019, -1019,
   -1019, -1019, -1019, -1019, -1019, -1019
  };

  const short
  Dhcp6Parser::yydefgoto_[] =
  {
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    83,    39,    40,    69,
     740,    87,    88,    41,    68,    84,    85,   770,   992,  1106,
    1107,   839,    43,    70,    90,    91,    92,   434,    45,    71,
     156,   157,   158,   437,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     461,   733,   175,   462,   176,   463,   177,   486,   178,   487,
     179,   180,   181,   182,   455,   183,   439,    47,    72,   222,
     223,   224,   501,   225,   184,   440,   185,   441,   186,   442,
     867,   868,   869,  1034,   840,   841,   842,  1009,  1257,   843,
    1010,   844,  1011,   845,  1012,   846,   847,   541,   848,   849,
     850,   851,   852,   853,   854,   855,   856,   857,  1018,   858,
     859,  1021,   860,  1022,   861,  1023,   187,   475,   901,   902,
     903,  1054,   188,   472,   888,   889,   890,   891,   189,   474,
     896,   897,   898,   899,   190,   473,   191,   482,   949,   950,
     951,   952,   953,   192,   478,   912,   913,   914,  1063,    63,
      80,   381,   382,   383,   554,   384,   555,   385,   556,   193,
     479,   921,   922,   923,   924,   925,   926,   927,   928,   194,
     465,   871,   872,   873,  1037,    49,    73,   260,   261,   262,
     507,   263,   508,   264,   509,   265,   513,   266,   512,   195,
     470,   746,   268,   269,   196,   471,   883,   884,   885,  1046,
    1177,  1178,   197,   466,    57,    77,   875,   876,   877,  1040,
      59,    78,   343,   344,   345,   346,   347,   348,   349,   540,
     350,   544,   351,   543,   352,   353,   545,   354,   198,   467,
     879,   880,   881,  1043,    61,    79,   364,   365,   366,   367,
     368,   549,   369,   370,   371,   372,   271,   505,   994,   995,
     996,  1108,    51,    74,   285,   286,   287,   518,   199,   468,
     200,   469,   274,   506,   998,   999,  1000,  1111,    53,    75,
     304,   305,   306,   522,   307,   308,   524,   309,   310,   201,
     477,   908,   909,   910,  1060,    55,    76,   323,   324,   325,
     326,   531,   327,   532,   328,   533,   329,   534,   330,   535,
     331,   536,   332,   530,   276,   514,  1003,  1004,  1114,   202,
     476,   905,   906,  1057,  1195,  1196,  1197,  1198,  1199,  1274,
    1200,   203,   480,   938,   939,   940,  1074,  1283,   941,   942,
    1075,   943,   944,   204,   205,   483,   961,   962,   987,   988,
     963,  1086,   964,  1087,   991,   206,   484,   971,   972,   973,
     974,  1091,   975,   976,  1093,   207,   485,    65,    81,   406,
     407,   408,   409,   562,   410,   563,   411,   412,   565,   413,
     414,   415,   568,   806,   416,   569,   417,   418,   419,   572,
     420,   573,   421,   574,   422,   575,   208,   438,    67,    82,
     425,   426,   427,   578,   428,   209,   488,   979,   980,  1097,
    1238,  1239,  1240,  1241,  1291,  1242,  1289,  1310,  1311,  1312,
    1320,  1321,  1322,  1328,  1323,  1324,  1325,  1326,  1332,   277,
     515,   210,   489,   211,   490,   212,   491,   386,   557,   213,
     492,   290,   311,   862,   214,   493,   215,   216,   387,   558,
     217,   218,   863,  1029,   864,  1030
  };

  const short
  Dhcp6Parser::yytable_[] =
  {
     155,   221,   239,   280,   297,   319,   989,   341,   360,   380,
     403,   342,   361,   362,   363,   990,   275,   226,   272,   288,
     302,   321,   935,   355,   373,    38,   404,   895,  1170,  1171,
    1172,   240,  1176,   241,  1182,   886,   242,   243,    31,    86,
      32,   739,    33,   865,   929,   804,   805,   244,   245,   423,
     424,   313,   246,   822,   247,   375,   376,   377,  1001,   435,
     122,   219,   220,   499,   436,   283,   300,    30,   500,    89,
     227,   273,   289,   303,   322,   503,   356,   374,   516,   405,
     504,   248,   981,   517,   429,   982,   249,   270,   284,   301,
     320,   776,    42,   124,   125,   281,   298,   250,   334,    44,
     251,    46,   252,   124,   125,   253,   783,   784,   785,   124,
     125,  1049,   254,   520,  1050,   255,   256,   257,   521,   739,
     258,   378,   335,   259,   528,   282,   299,   267,   559,   529,
     379,   124,   125,   560,    93,    94,   887,    48,    95,   124,
     125,    96,    97,    98,   124,   125,   335,    50,   336,   337,
     956,   957,   338,   339,   340,    52,   930,   931,   932,   933,
     956,   957,  1052,   124,   125,  1053,   430,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,  1280,  1281,  1282,   123,   335,   945,   946,   947,
     335,   357,   336,   337,   358,   359,   576,   580,   983,   124,
     125,   577,   581,   124,   125,    54,   126,   124,   125,    56,
     154,   127,   128,   129,   130,   131,  1231,   132,  1232,  1233,
     154,    58,   133,  1188,  1189,   499,   154,   375,   376,   377,
    1006,   134,    60,    62,    64,   135,    34,    35,    36,    37,
      66,   580,   136,  1170,  1171,  1172,  1007,  1176,   154,  1182,
     137,   138,   123,   124,   125,   139,   154,  1058,   140,   503,
    1059,   154,   141,   431,  1008,   291,   292,   293,   294,   295,
     124,   125,   432,  1315,   433,   233,  1316,  1317,  1318,  1319,
     154,  1031,  1031,   142,   143,   144,  1032,  1033,  1098,   235,
     443,  1099,   236,   378,   444,   145,   146,   147,  1313,   148,
     445,  1314,   379,   149,   446,   150,   151,  1055,   152,   153,
     447,  1072,  1056,   965,   966,   967,  1073,  1079,  1084,   895,
    1088,   154,  1080,  1085,  1094,  1089,   154,   448,   576,  1095,
     154,   989,   449,  1096,   154,   935,   728,   729,   730,   731,
     990,  1031,  1140,  1031,  1103,   817,  1100,   450,  1101,  1104,
     818,   819,   820,   821,   822,   823,   824,   825,   826,   827,
     828,   829,   830,   831,   832,   833,   834,   835,   451,   452,
     123,   453,   296,   915,   916,   917,   918,   919,   920,   454,
     968,   456,  1031,   538,   123,   516,   732,  1267,   124,   125,
    1268,  1271,   537,   335,   559,   278,  1272,   154,   457,  1279,
    1292,   539,   124,   125,   520,  1293,   312,   233,   458,  1295,
     459,   460,   313,   314,   315,   316,   317,   318,   155,   464,
     528,   235,   221,  1333,   236,  1296,   481,   494,  1334,   582,
     583,   742,   743,   744,   745,   239,   495,   496,   226,   280,
    1253,  1254,  1255,  1256,   497,   886,   893,   297,   894,   275,
     498,   272,   502,   510,   511,   288,   519,   319,   523,   525,
     526,   527,   542,   302,   240,   546,   241,   341,   547,   242,
     243,   342,   360,   321,   548,   552,   361,   362,   363,   550,
     244,   245,   551,   355,   553,   246,   561,   247,   373,   564,
     566,   227,   567,   570,   571,   403,   579,   584,   585,   587,
     588,   283,   589,   279,   273,   590,   591,   592,   289,   300,
     601,   404,   593,   836,   248,   154,   303,   594,   605,   249,
     270,   837,   838,   595,   284,   596,   322,   597,   608,   154,
     250,   281,   301,   251,   598,   252,   356,   154,   253,   298,
     599,   374,   320,   600,   604,   254,   602,   603,   255,   256,
     257,   609,   610,   258,   611,   612,   259,   613,   606,   607,
     267,   282,   614,   615,   405,   616,   617,   618,   619,   299,
     620,   621,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   622,   623,   624,   625,   114,   115,   116,
     117,   118,   119,   120,   626,   627,   123,   124,   125,   628,
     629,   630,   632,   631,   633,   634,   228,   635,   229,   636,
     637,   638,   639,   640,   124,   125,   230,   231,   232,   233,
     234,   126,   641,   642,   643,   644,   649,   645,   646,   652,
     647,   653,   654,   235,   655,   650,   236,   133,   656,   155,
     657,   658,   221,   659,   237,   660,   661,   662,   664,   665,
     667,   668,   669,   670,   674,   671,   672,   675,   226,   676,
     677,   388,   389,   390,   391,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   678,   934,   948,   958,   679,   403,
     680,   401,   402,   682,   683,   685,   684,   984,   686,   687,
     688,   936,   954,   959,   969,   404,   799,   800,   142,   143,
     690,   694,   695,   985,   696,   697,   698,   691,   701,   238,
     702,   227,   704,   692,   703,   700,   707,   720,   813,   705,
     706,   708,   711,   709,   154,     1,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,   710,
     712,   154,   713,   714,   937,   955,   960,   970,   405,   716,
     717,   719,   721,   722,   726,   723,   986,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   724,   725,
     727,   734,   114,   115,   116,   117,   118,   119,   120,   121,
     736,   123,   335,   735,   737,   738,   741,   747,    32,   748,
     749,   750,   900,   752,   751,   753,   754,   761,   762,   124,
     125,   755,   231,   232,   756,   234,   126,   757,   758,   759,
     760,   763,   764,   769,   767,   765,   768,   766,   235,   771,
     772,   236,   807,   773,   774,   775,   777,   778,   779,   237,
     780,   812,   866,  1013,   781,   870,   782,   786,   874,   787,
     878,   882,   788,   904,   789,   790,   791,   907,   792,   793,
     794,   795,   911,   796,   978,   993,   997,  1014,   797,  1015,
     801,  1016,   802,  1017,   803,   809,  1019,   810,  1020,  1024,
    1025,   811,  1026,  1027,  1028,  1036,  1105,  1035,  1039,  1038,
    1041,  1042,  1044,   142,   143,  1045,  1047,  1048,  1122,  1051,
    1062,  1061,  1064,  1065,  1109,  1066,  1067,  1068,  1069,  1070,
    1071,  1076,  1077,  1078,  1081,  1082,  1083,  1090,  1092,  1102,
    1110,  1113,  1112,  1121,   798,  1117,  1116,  1115,  1118,  1119,
    1120,  1123,  1126,  1129,  1130,  1124,   154,  1131,  1125,  1127,
    1128,  1137,  1138,  1132,  1133,  1186,  1135,  1134,  1136,  1206,
    1207,  1208,  1209,  1210,  1211,  1213,  1214,  1215,  1216,  1217,
    1223,  1224,  1227,  1219,  1220,  1221,   239,  1228,  1226,   341,
    1229,  1246,   360,   342,  1247,  1148,   361,   362,   363,  1169,
     275,  1244,   272,  1252,  1269,   355,  1190,  1258,   373,   319,
    1191,  1180,   380,  1259,  1260,   240,  1261,   241,  1270,  1262,
     242,   243,  1193,  1275,  1149,   321,  1150,  1277,   934,  1151,
    1152,   244,   245,   948,  1276,  1263,   246,   958,   247,  1264,
    1153,  1154,  1265,  1278,   936,  1155,  1234,  1156,  1290,   954,
    1235,  1299,   984,   959,  1266,   273,  1273,   280,   356,   969,
     297,   374,  1236,  1300,  1181,   248,  1284,  1285,   985,  1302,
     249,   270,  1304,   288,  1157,  1194,   302,  1307,   322,  1158,
    1179,   250,  1286,  1309,   251,  1287,   252,  1173,  1303,   253,
    1159,  1192,  1329,  1160,   320,  1161,   254,   937,  1162,   255,
     256,   257,   955,  1294,   258,  1163,   960,   259,  1164,  1165,
    1166,   267,   970,  1167,  1330,  1237,  1168,  1174,  1331,   283,
    1175,   986,   300,  1335,  1297,  1336,   289,  1339,   892,   303,
     586,   814,  1306,   808,  1005,  1141,   816,  1139,  1187,  1184,
    1185,  1222,   284,   648,   699,   301,  1205,  1204,  1143,   281,
    1142,  1183,   298,  1298,   333,  1308,  1144,  1337,  1212,  1145,
    1147,  1338,   663,  1146,  1341,  1342,   689,  1251,  1249,   693,
    1203,  1250,  1301,  1218,  1248,  1245,  1002,  1201,  1202,   282,
     977,  1225,   299,  1230,   715,   718,   815,  1327,  1305,  1243,
    1340,     0,   651,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     666,     0,     0,     0,     0,     0,     0,     0,     0,   673,
    1148,     0,     0,     0,  1169,     0,  1190,     0,     0,     0,
    1191,     0,   681,     0,     0,     0,  1180,     0,     0,     0,
       0,  1234,  1193,     0,     0,  1235,     0,     0,     0,  1149,
       0,  1150,     0,     0,  1151,  1152,     0,  1236,  1288,     0,
       0,     0,     0,     0,     0,  1153,  1154,     0,     0,     0,
    1155,     0,  1156,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1181,
       0,     0,     0,     0,     0,  1194,     0,     0,     0,  1157,
       0,     0,     0,     0,  1158,  1179,     0,     0,     0,     0,
    1237,  1192,  1173,     0,     0,  1159,     0,     0,  1160,     0,
    1161,     0,     0,  1162,     0,     0,     0,     0,     0,     0,
    1163,     0,     0,  1164,  1165,  1166,     0,     0,  1167,     0,
       0,  1168,  1174,     0,     0,  1175
  };

  const short
  Dhcp6Parser::yycheck_[] =
  {
      71,    72,    73,    74,    75,    76,   768,    78,    79,    80,
      81,    78,    79,    79,    79,   768,    73,    72,    73,    74,
      75,    76,   756,    78,    79,    16,    81,   750,  1046,  1046,
    1046,    73,  1046,    73,  1046,   112,    73,    73,     5,   213,
       7,   618,     9,   725,    21,   163,   164,    73,    73,    13,
      14,   110,    73,    30,    73,   119,   120,   121,   117,     3,
      67,    16,    17,     3,     8,    74,    75,     0,     8,    10,
      72,    73,    74,    75,    76,     3,    78,    79,     3,    81,
       8,    73,   764,     8,     6,   767,    73,    73,    74,    75,
      76,   659,     7,    86,    87,    74,    75,    73,    21,     7,
      73,     7,    73,    86,    87,    73,   674,   675,   676,    86,
      87,     3,    73,     3,     6,    73,    73,    73,     8,   696,
      73,   185,    69,    73,     3,    74,    75,    73,     3,     8,
     194,    86,    87,     8,    11,    12,   213,     7,    15,    86,
      87,    18,    19,    20,    86,    87,    69,     7,    71,    72,
     143,   144,    75,    76,    77,     7,   133,   134,   135,   136,
     143,   144,     3,    86,    87,     6,     3,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,   130,   131,   132,    68,    69,   139,   140,   141,
      69,    70,    71,    72,    73,    74,     3,     3,   191,    86,
      87,     8,     8,    86,    87,     7,    93,    86,    87,     7,
     213,    98,    99,   100,   101,   102,   173,   104,   175,   176,
     213,     7,   109,   106,   107,     3,   213,   119,   120,   121,
       8,   118,     7,     7,     7,   122,   213,   214,   215,   216,
       7,     3,   129,  1271,  1271,  1271,     8,  1271,   213,  1271,
     137,   138,    68,    86,    87,   142,   213,     3,   145,     3,
       6,   213,   149,     4,     8,    81,    82,    83,    84,    85,
      86,    87,     8,   174,     3,    91,   177,   178,   179,   180,
     213,     3,     3,   170,   171,   172,     8,     8,     3,   105,
       4,     6,   108,   185,     4,   182,   183,   184,     3,   186,
       4,     6,   194,   190,     4,   192,   193,     3,   195,   196,
       4,     3,     8,   146,   147,   148,     8,     3,     3,  1052,
       3,   213,     8,     8,     3,     8,   213,     4,     3,     8,
     213,  1103,     4,     8,   213,  1079,   166,   167,   168,   169,
    1103,     3,  1034,     3,     3,    21,     8,     4,     8,     8,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     4,     4,
      68,     4,   188,   123,   124,   125,   126,   127,   128,     4,
     213,     4,     3,     3,    68,     3,   216,     8,    86,    87,
       8,     3,     8,    69,     3,    79,     8,   213,     4,     8,
       3,     8,    86,    87,     3,     8,   104,    91,     4,     8,
       4,     4,   110,   111,   112,   113,   114,   115,   499,     4,
       3,   105,   503,     3,   108,     8,     4,     4,     8,   430,
     431,    94,    95,    96,    97,   516,     4,     4,   503,   520,
      22,    23,    24,    25,     4,   112,   113,   528,   115,   516,
       4,   516,     4,     4,     4,   520,     4,   538,     4,     4,
       4,     4,     4,   528,   516,     4,   516,   548,     8,   516,
     516,   548,   553,   538,     3,     8,   553,   553,   553,     4,
     516,   516,     4,   548,     3,   516,     4,   516,   553,     4,
       4,   503,     4,     4,     4,   576,     4,   213,     4,     4,
       4,   520,     4,   187,   516,     4,     4,     4,   520,   528,
     216,   576,   214,   189,   516,   213,   528,   214,     4,   516,
     516,   197,   198,   214,   520,   214,   538,   214,   216,   213,
     516,   520,   528,   516,   214,   516,   548,   213,   516,   528,
     214,   553,   538,   214,   214,   516,   215,   215,   516,   516,
     516,   216,   216,   516,     4,     4,   516,     4,   214,   214,
     516,   520,   216,     4,   576,     4,     4,     4,     4,   528,
       4,     4,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,     4,     4,     4,     4,    59,    60,    61,
      62,    63,    64,    65,     4,     4,    68,    86,    87,     4,
       4,     4,     4,   214,     4,     4,    78,     4,    80,     4,
       4,     4,     4,     4,    86,    87,    88,    89,    90,    91,
      92,    93,     4,     4,     4,   213,     4,   214,   214,     4,
     214,     4,     4,   105,     4,   216,   108,   109,     4,   720,
     214,   216,   723,     4,   116,     4,     4,     4,     4,   214,
       4,   214,     4,   214,     4,   214,   214,     4,   723,     4,
       4,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,     4,   756,   757,   758,     4,   760,
       4,   170,   171,     4,     4,     4,   214,   768,     4,     4,
     216,   756,   757,   758,   759,   760,   697,   698,   170,   171,
       4,     4,     4,   768,     4,     4,     4,   216,     4,   181,
       4,   723,     4,   216,   214,   216,     4,     7,   719,   214,
     214,     4,     4,   216,   213,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   216,
       4,   213,     4,     4,   756,   757,   758,   759,   760,     4,
     214,     4,   213,     7,     5,     7,   768,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,     7,     7,
     213,   213,    59,    60,    61,    62,    63,    64,    65,    66,
       5,    68,    69,   213,     5,     5,   213,     5,     7,     5,
       5,     5,   103,     5,     7,     5,     5,   213,   213,    86,
      87,     7,    89,    90,     7,    92,    93,     7,     7,     7,
       7,     5,     7,     5,     7,   213,     7,   213,   105,     5,
       5,   108,   165,   213,   213,   213,   213,     7,   213,   116,
     213,     5,     7,     4,   213,     7,   213,   213,     7,   213,
       7,     7,   213,     7,   213,   213,   213,     7,   213,   213,
     213,   213,     7,   213,     7,     7,     7,     4,   213,     4,
     213,     4,   213,     4,   213,   213,     4,   213,     4,     4,
       4,   213,     4,     4,     4,     3,   213,     6,     3,     6,
       6,     3,     6,   170,   171,     3,     6,     3,   216,     6,
       3,     6,     6,     3,     6,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       3,     3,     6,   214,   696,     4,     6,     8,     4,     4,
       4,   214,     4,     4,     4,   216,   213,     4,   214,   214,
     214,     4,     4,   214,   214,     4,   214,   216,   214,   214,
     214,   214,   214,   214,   214,     4,     4,   214,   214,   214,
       4,     4,     4,   216,   214,   214,  1037,   214,   216,  1040,
       4,     6,  1043,  1040,     3,  1046,  1043,  1043,  1043,  1046,
    1037,   216,  1037,     4,     8,  1040,  1057,   213,  1043,  1060,
    1057,  1046,  1063,   213,   213,  1037,   213,  1037,     8,   213,
    1037,  1037,  1057,     4,  1046,  1060,  1046,     3,  1079,  1046,
    1046,  1037,  1037,  1084,     8,   213,  1037,  1088,  1037,   213,
    1046,  1046,   213,     8,  1079,  1046,  1097,  1046,     4,  1084,
    1097,     4,  1103,  1088,   213,  1037,   213,  1108,  1040,  1094,
    1111,  1043,  1097,   216,  1046,  1037,   213,   213,  1103,     4,
    1037,  1037,     4,  1108,  1046,  1057,  1111,     5,  1060,  1046,
    1046,  1037,   213,     7,  1037,   213,  1037,  1046,   214,  1037,
    1046,  1057,     4,  1046,  1060,  1046,  1037,  1079,  1046,  1037,
    1037,  1037,  1084,   213,  1037,  1046,  1088,  1037,  1046,  1046,
    1046,  1037,  1094,  1046,     4,  1097,  1046,  1046,     4,  1108,
    1046,  1103,  1111,     4,   213,   216,  1108,     4,   749,  1111,
     435,   720,   213,   711,   812,  1036,   723,  1031,  1055,  1049,
    1052,  1084,  1108,   499,   559,  1111,  1065,  1063,  1039,  1108,
    1037,  1048,  1111,  1271,    77,   213,  1040,   214,  1072,  1042,
    1045,   214,   516,  1043,   213,   213,   548,  1113,  1110,   553,
    1062,  1111,  1277,  1079,  1108,  1103,   778,  1058,  1060,  1108,
     760,  1088,  1111,  1094,   576,   580,   722,  1313,  1292,  1098,
    1333,    -1,   503,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     520,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   528,
    1271,    -1,    -1,    -1,  1271,    -1,  1277,    -1,    -1,    -1,
    1277,    -1,   538,    -1,    -1,    -1,  1271,    -1,    -1,    -1,
      -1,  1292,  1277,    -1,    -1,  1292,    -1,    -1,    -1,  1271,
      -1,  1271,    -1,    -1,  1271,  1271,    -1,  1292,  1229,    -1,
      -1,    -1,    -1,    -1,    -1,  1271,  1271,    -1,    -1,    -1,
    1271,    -1,  1271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1271,
      -1,    -1,    -1,    -1,    -1,  1277,    -1,    -1,    -1,  1271,
      -1,    -1,    -1,    -1,  1271,  1271,    -1,    -1,    -1,    -1,
    1292,  1277,  1271,    -1,    -1,  1271,    -1,    -1,  1271,    -1,
    1271,    -1,    -1,  1271,    -1,    -1,    -1,    -1,    -1,    -1,
    1271,    -1,    -1,  1271,  1271,  1271,    -1,    -1,  1271,    -1,
      -1,  1271,  1271,    -1,    -1,  1271
  };

  const short
  Dhcp6Parser::yystos_[] =
  {
       0,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
       0,     5,     7,     9,   213,   214,   215,   216,   233,   234,
     235,   240,     7,   249,     7,   255,     7,   294,     7,   402,
       7,   479,     7,   495,     7,   512,     7,   431,     7,   437,
       7,   461,     7,   376,     7,   584,     7,   615,   241,   236,
     250,   256,   295,   403,   480,   496,   513,   432,   438,   462,
     377,   585,   616,   233,   242,   243,   213,   238,   239,    10,
     251,   252,   253,    11,    12,    15,    18,    19,    20,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    86,    87,    93,    98,    99,   100,
     101,   102,   104,   109,   118,   122,   129,   137,   138,   142,
     145,   149,   170,   171,   172,   182,   183,   184,   186,   190,
     192,   193,   195,   196,   213,   248,   257,   258,   259,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   279,   281,   283,   285,   287,
     288,   289,   290,   292,   301,   303,   305,   343,   349,   355,
     361,   363,   370,   386,   396,   416,   421,   429,   455,   485,
     487,   506,   536,   548,   560,   561,   572,   582,   613,   622,
     648,   650,   652,   656,   661,   663,   664,   667,   668,    16,
      17,   248,   296,   297,   298,   300,   485,   487,    78,    80,
      88,    89,    90,    91,    92,   105,   108,   116,   181,   248,
     261,   262,   263,   264,   265,   266,   267,   268,   269,   270,
     271,   273,   274,   275,   276,   279,   281,   283,   285,   287,
     404,   405,   406,   408,   410,   412,   414,   416,   419,   420,
     455,   473,   485,   487,   489,   506,   531,   646,    79,   187,
     248,   412,   414,   419,   455,   481,   482,   483,   485,   487,
     658,    81,    82,    83,    84,    85,   188,   248,   412,   414,
     419,   455,   485,   487,   497,   498,   499,   501,   502,   504,
     505,   659,   104,   110,   111,   112,   113,   114,   115,   248,
     455,   485,   487,   514,   515,   516,   517,   519,   521,   523,
     525,   527,   529,   429,    21,    69,    71,    72,    75,    76,
      77,   248,   323,   439,   440,   441,   442,   443,   444,   445,
     447,   449,   451,   452,   454,   485,   487,    70,    73,    74,
     248,   323,   443,   449,   463,   464,   465,   466,   467,   469,
     470,   471,   472,   485,   487,   119,   120,   121,   185,   194,
     248,   378,   379,   380,   382,   384,   654,   665,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   170,   171,   248,   485,   487,   586,   587,   588,   589,
     591,   593,   594,   596,   597,   598,   601,   603,   604,   605,
     607,   609,   611,    13,    14,   617,   618,   619,   621,     6,
       3,     4,     8,     3,   254,     3,     8,   260,   614,   293,
     302,   304,   306,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,   291,     4,     4,     4,     4,
       4,   277,   280,   282,     4,   397,   430,   456,   486,   488,
     417,   422,   350,   362,   356,   344,   537,   507,   371,   387,
     549,     4,   364,   562,   573,   583,   284,   286,   623,   649,
     651,   653,   657,   662,     4,     4,     4,     4,     4,     3,
       8,   299,     4,     3,     8,   474,   490,   407,   409,   411,
       4,     4,   415,   413,   532,   647,     3,     8,   484,     4,
       3,     8,   500,     4,   503,     4,     4,     4,     3,     8,
     530,   518,   520,   522,   524,   526,   528,     8,     3,     8,
     446,   324,     4,   450,   448,   453,     4,     8,     3,   468,
       4,     4,     8,     3,   381,   383,   385,   655,   666,     3,
       8,     4,   590,   592,     4,   595,     4,     4,   599,   602,
       4,     4,   606,   608,   610,   612,     3,     8,   620,     4,
       3,     8,   233,   233,   213,     4,   252,     4,     4,     4,
       4,     4,     4,   214,   214,   214,   214,   214,   214,   214,
     214,   216,   215,   215,   214,     4,   214,   214,   216,   216,
     216,     4,     4,     4,   216,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,   214,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,   213,   214,   214,   214,   258,     4,
     216,   297,     4,     4,     4,     4,     4,   214,   216,     4,
       4,     4,     4,   405,     4,   214,   482,     4,   214,     4,
     214,   214,   214,   498,     4,     4,     4,     4,     4,     4,
       4,   516,     4,     4,   214,     4,     4,     4,   216,   441,
       4,   216,   216,   465,     4,     4,     4,     4,     4,   379,
     216,     4,     4,   214,     4,   214,   214,     4,     4,   216,
     216,     4,     4,     4,     4,   587,     4,   214,   618,     4,
       7,   213,     7,     7,     7,     7,     5,   213,   166,   167,
     168,   169,   216,   278,   213,   213,     5,     5,     5,   235,
     237,   213,    94,    95,    96,    97,   418,     5,     5,     5,
       5,     7,     5,     5,     5,     7,     7,     7,     7,     7,
       7,   213,   213,     5,     7,   213,   213,     7,     7,     5,
     244,     5,     5,   213,   213,   213,   244,   213,     7,   213,
     213,   213,   213,   244,   244,   244,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   237,   233,
     233,   213,   213,   213,   163,   164,   600,   165,   278,   213,
     213,   213,     5,   233,   257,   617,   296,    21,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,   189,   197,   198,   248,
     311,   312,   313,   316,   318,   320,   322,   323,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   336,   337,
     339,   341,   660,   669,   671,   311,     7,   307,   308,   309,
       7,   398,   399,   400,     7,   433,   434,   435,     7,   457,
     458,   459,     7,   423,   424,   425,   112,   213,   351,   352,
     353,   354,   242,   113,   115,   353,   357,   358,   359,   360,
     103,   345,   346,   347,     7,   538,   539,     7,   508,   509,
     510,     7,   372,   373,   374,   123,   124,   125,   126,   127,
     128,   388,   389,   390,   391,   392,   393,   394,   395,    21,
     133,   134,   135,   136,   248,   325,   485,   487,   550,   551,
     552,   555,   556,   558,   559,   139,   140,   141,   248,   365,
     366,   367,   368,   369,   485,   487,   143,   144,   248,   485,
     487,   563,   564,   567,   569,   146,   147,   148,   213,   485,
     487,   574,   575,   576,   577,   579,   580,   586,     7,   624,
     625,   311,   311,   191,   248,   485,   487,   565,   566,   567,
     569,   571,   245,     7,   475,   476,   477,     7,   491,   492,
     493,   117,   517,   533,   534,   307,     8,     8,     8,   314,
     317,   319,   321,     4,     4,     4,     4,     4,   335,     4,
       4,   338,   340,   342,     4,     4,     4,     4,     4,   670,
     672,     3,     8,     8,   310,     6,     3,   401,     6,     3,
     436,     6,     3,   460,     6,     3,   426,     6,     3,     3,
       6,     6,     3,     6,   348,     3,     8,   540,     3,     6,
     511,     6,     3,   375,     6,     3,     4,     4,     4,     4,
       4,     4,     3,     8,   553,   557,     4,     4,     4,     3,
       8,     4,     4,     4,     3,     8,   568,   570,     3,     8,
       4,   578,     4,   581,     3,     8,     8,   626,     3,     6,
       8,     8,     4,     3,     8,   213,   246,   247,   478,     6,
       3,   494,     6,     3,   535,     8,     6,     4,     4,     4,
       4,   214,   216,   214,   216,   214,     4,   214,   214,     4,
       4,     4,   214,   214,   216,   214,   214,     4,     4,   312,
     311,   309,   404,   400,   439,   435,   463,   459,   248,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     273,   274,   275,   276,   279,   281,   283,   285,   287,   323,
     396,   408,   410,   412,   414,   416,   420,   427,   428,   455,
     485,   487,   531,   425,   352,   358,     4,   346,   106,   107,
     248,   323,   455,   485,   487,   541,   542,   543,   544,   545,
     547,   539,   514,   510,   378,   374,   214,   214,   214,   214,
     214,   214,   389,     4,     4,   214,   214,   214,   551,   216,
     214,   214,   366,     4,     4,   564,   216,     4,   214,     4,
     575,   173,   175,   176,   248,   323,   485,   487,   627,   628,
     629,   630,   632,   625,   216,   566,     6,     3,   481,   477,
     497,   493,     4,    22,    23,    24,    25,   315,   213,   213,
     213,   213,   213,   213,   213,   213,   213,     8,     8,     8,
       8,     3,     8,   213,   546,     4,     8,     3,     8,     8,
     130,   131,   132,   554,   213,   213,   213,   213,   233,   633,
       4,   631,     3,     8,   213,     8,     8,   213,   428,     4,
     216,   543,     4,   214,     4,   628,   213,     5,   213,     7,
     634,   635,   636,     3,     6,   174,   177,   178,   179,   180,
     637,   638,   639,   641,   642,   643,   644,   635,   640,     4,
       4,     4,   645,     3,     8,     4,   216,   214,   214,     4,
     638,   213,   213
  };

  const short
  Dhcp6Parser::yyr1_[] =
  {
       0,   217,   219,   218,   220,   218,   221,   218,   222,   218,
     223,   218,   224,   218,   225,   218,   226,   218,   227,   218,
     228,   218,   229,   218,   230,   218,   231,   218,   232,   218,
     233,   233,   233,   233,   233,   233,   233,   234,   236,   235,
     237,   238,   238,   239,   239,   241,   240,   242,   242,   243,
     243,   245,   244,   246,   246,   247,   247,   248,   250,   249,
     251,   251,   252,   254,   253,   256,   255,   257,   257,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   258,   258,   258,   258,   258,   258,   258,
     258,   260,   259,   261,   262,   263,   264,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   277,   276,
     278,   278,   278,   278,   278,   280,   279,   282,   281,   284,
     283,   286,   285,   287,   288,   289,   291,   290,   293,   292,
     295,   294,   296,   296,   297,   297,   297,   297,   297,   299,
     298,   300,   302,   301,   304,   303,   306,   305,   307,   307,
     308,   308,   310,   309,   311,   311,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     314,   313,   315,   315,   315,   315,   317,   316,   319,   318,
     321,   320,   322,   324,   323,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   335,   334,   336,   338,   337,   340,
     339,   342,   341,   344,   343,   345,   345,   346,   348,   347,
     350,   349,   351,   351,   352,   352,   353,   354,   356,   355,
     357,   357,   358,   358,   358,   359,   360,   362,   361,   364,
     363,   365,   365,   366,   366,   366,   366,   366,   366,   367,
     368,   369,   371,   370,   372,   372,   373,   373,   375,   374,
     377,   376,   378,   378,   378,   379,   379,   379,   379,   379,
     381,   380,   383,   382,   385,   384,   387,   386,   388,   388,
     389,   389,   389,   389,   389,   389,   390,   391,   392,   393,
     394,   395,   397,   396,   398,   398,   399,   399,   401,   400,
     403,   402,   404,   404,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   407,   406,   409,   408,   411,   410,   413,   412,   415,
     414,   417,   416,   418,   418,   418,   418,   419,   420,   422,
     421,   423,   423,   424,   424,   426,   425,   427,   427,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   428,   430,   429,   432,   431,   433,   433,   434,   434,
     436,   435,   438,   437,   439,   439,   440,   440,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   442,   443,
     444,   446,   445,   448,   447,   450,   449,   451,   453,   452,
     454,   456,   455,   457,   457,   458,   458,   460,   459,   462,
     461,   463,   463,   464,   464,   465,   465,   465,   465,   465,
     465,   465,   465,   465,   466,   468,   467,   469,   470,   471,
     472,   474,   473,   475,   475,   476,   476,   478,   477,   480,
     479,   481,   481,   482,   482,   482,   482,   482,   482,   482,
     482,   482,   484,   483,   486,   485,   488,   487,   490,   489,
     491,   491,   492,   492,   494,   493,   496,   495,   497,   497,
     498,   498,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   500,   499,   501,   503,   502,   504,   505,
     507,   506,   508,   508,   509,   509,   511,   510,   513,   512,
     514,   514,   515,   515,   516,   516,   516,   516,   516,   516,
     516,   516,   516,   516,   516,   518,   517,   520,   519,   522,
     521,   524,   523,   526,   525,   528,   527,   530,   529,   532,
     531,   533,   533,   535,   534,   537,   536,   538,   538,   540,
     539,   541,   541,   542,   542,   543,   543,   543,   543,   543,
     543,   543,   544,   546,   545,   547,   549,   548,   550,   550,
     551,   551,   551,   551,   551,   551,   551,   551,   551,   553,
     552,   554,   554,   554,   555,   557,   556,   558,   559,   560,
     562,   561,   563,   563,   564,   564,   564,   564,   564,   565,
     565,   566,   566,   566,   566,   566,   566,   568,   567,   570,
     569,   571,   573,   572,   574,   574,   575,   575,   575,   575,
     575,   575,   576,   578,   577,   579,   581,   580,   583,   582,
     585,   584,   586,   586,   587,   587,   587,   587,   587,   587,
     587,   587,   587,   587,   587,   587,   587,   587,   587,   587,
     587,   587,   588,   590,   589,   592,   591,   593,   595,   594,
     596,   597,   599,   598,   600,   600,   602,   601,   603,   604,
     606,   605,   608,   607,   610,   609,   612,   611,   614,   613,
     616,   615,   617,   617,   618,   618,   620,   619,   621,   623,
     622,   624,   624,   626,   625,   627,   627,   628,   628,   628,
     628,   628,   628,   628,   629,   631,   630,   633,   632,   634,
     634,   636,   635,   637,   637,   638,   638,   638,   638,   638,
     640,   639,   641,   642,   643,   645,   644,   647,   646,   649,
     648,   651,   650,   653,   652,   655,   654,   657,   656,   658,
     659,   660,   662,   661,   663,   664,   666,   665,   667,   668,
     670,   669,   672,   671
  };

  const signed char
  Dhcp6Parser::yyr2_[] =
  {
       0,     2,     0,     3,     0,     3,     0,     3,     0,     3,
       0,     3,     0,     3,     0,     3,     0,     3,     0,     3,
       0,     3,     0,     3,     0,     3,     0,     3,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     4,
       1,     0,     1,     3,     5,     0,     4,     0,     1,     1,
       3,     0,     4,     0,     1,     1,     3,     2,     0,     4,
       1,     3,     1,     0,     6,     0,     4,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     4,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     0,     4,
       1,     1,     1,     1,     1,     0,     4,     0,     4,     0,
       4,     0,     4,     3,     3,     3,     0,     4,     0,     6,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     0,
       4,     3,     0,     6,     0,     6,     0,     6,     0,     1,
       1,     3,     0,     4,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     4,     1,     1,     1,     1,     0,     4,     0,     4,
       0,     4,     3,     0,     4,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     0,     4,     3,     0,     4,     0,
       4,     0,     4,     0,     6,     1,     3,     1,     0,     4,
       0,     6,     1,     3,     1,     1,     1,     1,     0,     6,
       1,     3,     1,     1,     1,     1,     1,     0,     6,     0,
       6,     1,     3,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     0,     6,     0,     1,     1,     3,     0,     4,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       0,     4,     0,     4,     0,     4,     0,     6,     1,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     0,     6,     0,     1,     1,     3,     0,     4,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     4,     1,     1,     1,     1,     3,     3,     0,
       6,     0,     1,     1,     3,     0,     4,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     6,     0,     4,     0,     1,     1,     3,
       0,     4,     0,     4,     0,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     0,     4,     0,     4,     0,     4,     1,     0,     4,
       3,     0,     6,     0,     1,     1,     3,     0,     4,     0,
       4,     0,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     4,     1,     1,     3,
       3,     0,     6,     0,     1,     1,     3,     0,     4,     0,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     0,     4,     0,     4,     0,     6,
       0,     1,     1,     3,     0,     4,     0,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     4,     3,     0,     4,     3,     3,
       0,     6,     0,     1,     1,     3,     0,     4,     0,     4,
       0,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     4,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       6,     1,     1,     0,     4,     0,     6,     1,     3,     0,
       4,     0,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     4,     3,     0,     6,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       4,     1,     1,     1,     3,     0,     4,     3,     3,     3,
       0,     6,     1,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     0,     4,     0,
       4,     3,     0,     6,     1,     3,     1,     1,     1,     1,
       1,     1,     3,     0,     4,     3,     0,     4,     0,     6,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     0,     4,     0,     4,     3,     0,     4,
       3,     3,     0,     4,     1,     1,     0,     4,     3,     3,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     6,
       0,     4,     1,     3,     1,     1,     0,     6,     3,     0,
       6,     1,     3,     0,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     3,     0,     4,     0,     6,     1,
       3,     0,     4,     1,     3,     1,     1,     1,     1,     1,
       0,     4,     3,     3,     3,     0,     4,     0,     4,     0,
       6,     0,     4,     0,     4,     0,     4,     0,     6,     3,
       3,     3,     0,     6,     3,     3,     0,     4,     3,     3,
       0,     4,     0,     4
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const Dhcp6Parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\",\"", "\":\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"null\"", "\"Dhcp6\"", "\"data-directory\"",
  "\"config-control\"", "\"config-databases\"",
  "\"config-fetch-wait-time\"", "\"interfaces-config\"", "\"interfaces\"",
  "\"re-detect\"", "\"lease-database\"", "\"hosts-database\"",
  "\"hosts-databases\"", "\"type\"", "\"memfile\"", "\"mysql\"",
  "\"postgresql\"", "\"cql\"", "\"user\"", "\"password\"", "\"host\"",
  "\"port\"", "\"persist\"", "\"lfc-interval\"", "\"readonly\"",
  "\"connect-timeout\"", "\"contact-points\"", "\"max-reconnect-tries\"",
  "\"reconnect-wait-time\"", "\"keyspace\"", "\"consistency\"",
  "\"serial-consistency\"", "\"request-timeout\"", "\"tcp-keepalive\"",
  "\"tcp-nodelay\"", "\"max-row-errors\"", "\"preferred-lifetime\"",
  "\"min-preferred-lifetime\"", "\"max-preferred-lifetime\"",
  "\"valid-lifetime\"", "\"min-valid-lifetime\"", "\"max-valid-lifetime\"",
  "\"renew-timer\"", "\"rebind-timer\"", "\"calculate-tee-times\"",
  "\"t1-percent\"", "\"t2-percent\"", "\"decline-probation-period\"",
  "\"server-tag\"", "\"statistic-default-sample-count\"",
  "\"statistic-default-sample-age\"", "\"ddns-send-updates\"",
  "\"ddns-override-no-update\"", "\"ddns-override-client-update\"",
  "\"ddns-replace-client-name\"", "\"ddns-generated-prefix\"",
  "\"ddns-qualifying-suffix\"", "\"store-extended-info\"", "\"subnet6\"",
  "\"option-def\"", "\"option-data\"", "\"name\"", "\"data\"", "\"code\"",
  "\"space\"", "\"csv-format\"", "\"always-send\"", "\"record-types\"",
  "\"encapsulate\"", "\"array\"", "\"pools\"", "\"pool\"", "\"pd-pools\"",
  "\"prefix\"", "\"prefix-len\"", "\"excluded-prefix\"",
  "\"excluded-prefix-len\"", "\"delegated-len\"", "\"user-context\"",
  "\"comment\"", "\"subnet\"", "\"interface\"", "\"interface-id\"",
  "\"id\"", "\"rapid-commit\"", "\"reservation-mode\"", "\"disabled\"",
  "\"out-of-pool\"", "\"global\"", "\"all\"", "\"shared-networks\"",
  "\"mac-sources\"", "\"relay-supplied-options\"",
  "\"host-reservation-identifiers\"", "\"sanity-checks\"",
  "\"lease-checks\"", "\"client-classes\"", "\"require-client-classes\"",
  "\"test\"", "\"only-if-required\"", "\"client-class\"",
  "\"reservations\"", "\"ip-addresses\"", "\"prefixes\"", "\"duid\"",
  "\"hw-address\"", "\"hostname\"", "\"flex-id\"", "\"relay\"",
  "\"ip-address\"", "\"hooks-libraries\"", "\"library\"",
  "\"library-path\"", "\"parameters\"", "\"expired-leases-processing\"",
  "\"reclaim-timer-wait-time\"", "\"flush-reclaimed-timer-wait-time\"",
  "\"hold-reclaimed-time\"", "\"max-reclaim-leases\"",
  "\"max-reclaim-time\"", "\"unwarned-reclaim-cycles\"", "\"server-id\"",
  "\"LLT\"", "\"EN\"", "\"LL\"", "\"identifier\"", "\"htype\"", "\"time\"",
  "\"enterprise-id\"", "\"dhcp4o6-port\"", "\"multi-threading\"",
  "\"enable-multi-threading\"", "\"thread-pool-size\"",
  "\"packet-queue-size\"", "\"control-socket\"", "\"socket-type\"",
  "\"socket-name\"", "\"dhcp-queue-control\"", "\"enable-queue\"",
  "\"queue-type\"", "\"capacity\"", "\"dhcp-ddns\"", "\"enable-updates\"",
  "\"qualifying-suffix\"", "\"server-ip\"", "\"server-port\"",
  "\"sender-ip\"", "\"sender-port\"", "\"max-queue-size\"",
  "\"ncr-protocol\"", "\"ncr-format\"", "\"override-no-update\"",
  "\"override-client-update\"", "\"replace-client-name\"",
  "\"generated-prefix\"", "\"UDP\"", "\"TCP\"", "\"JSON\"",
  "\"when-present\"", "\"never\"", "\"always\"", "\"when-not-present\"",
  "\"hostname-char-set\"", "\"hostname-char-replacement\"", "\"loggers\"",
  "\"output_options\"", "\"output\"", "\"debuglevel\"", "\"severity\"",
  "\"flush\"", "\"maxsize\"", "\"maxver\"", "\"pattern\"",
  "\"allocation-type\"", "\"config-database\"", "\"configuration-type\"",
  "\"instance-id\"", "\"lawful-interception-parameters\"",
  "\"master-database\"", "\"max-address-utilization\"",
  "\"max-pd-space-utilization\"", "\"max-statement-tries\"",
  "\"notifications\"",
  "\"on-boot-request-configuration-from-kea-netconf\"",
  "\"option-set-description\"", "\"option-set-id\"",
  "\"policy-engine-parameters\"", "\"privacy-history-size\"",
  "\"privacy-valid-lifetime\"", "\"protocol\"", "\"ssl-cert\"",
  "TOPLEVEL_JSON", "TOPLEVEL_DHCP6", "SUB_DHCP6", "SUB_INTERFACES6",
  "SUB_SUBNET6", "SUB_POOL6", "SUB_PD_POOL", "SUB_RESERVATION",
  "SUB_OPTION_DEFS", "SUB_OPTION_DEF", "SUB_OPTION_DATA",
  "SUB_HOOKS_LIBRARY", "SUB_DHCP_DDNS", "SUB_CONFIG_CONTROL",
  "\"constant string\"", "\"integer\"", "\"floating point\"",
  "\"boolean\"", "$accept", "start", "$@1", "$@2", "$@3", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13", "$@14",
  "value", "sub_json", "map2", "$@15", "map_value", "map_content",
  "not_empty_map", "list_generic", "$@16", "list_content",
  "not_empty_list", "list_strings", "$@17", "list_strings_content",
  "not_empty_list_strings", "unknown_map_entry", "syntax_map", "$@18",
  "global_objects", "global_object", "dhcp6_object", "$@19", "sub_dhcp6",
  "$@20", "global_params", "global_param", "data_directory", "$@21",
  "preferred_lifetime", "min_preferred_lifetime", "max_preferred_lifetime",
  "valid_lifetime", "min_valid_lifetime", "max_valid_lifetime",
  "renew_timer", "rebind_timer", "calculate_tee_times", "t1_percent",
  "t2_percent", "decline_probation_period", "ddns_send_updates",
  "ddns_override_no_update", "ddns_override_client_update",
  "ddns_replace_client_name", "$@22", "ddns_replace_client_name_value",
  "ddns_generated_prefix", "$@23", "ddns_qualifying_suffix", "$@24",
  "hostname_char_set", "$@25", "hostname_char_replacement", "$@26",
  "store_extended_info", "statistic_default_sample_count",
  "statistic_default_sample_age", "server_tag", "$@27",
  "interfaces_config", "$@28", "sub_interfaces6", "$@29",
  "interfaces_config_params", "interfaces_config_param", "interfaces_list",
  "$@30", "re_detect", "lease_database", "$@31", "hosts_database", "$@32",
  "hosts_databases", "$@33", "database_list", "not_empty_database_list",
  "database", "$@34", "database_map_params", "database_map_param",
  "database_type", "$@35", "db_type", "user", "$@36", "password", "$@37",
  "host", "$@38", "port", "name", "$@39", "persist", "lfc_interval",
  "readonly", "connect_timeout", "reconnect_wait_time", "max_row_errors",
  "request_timeout", "tcp_keepalive", "tcp_nodelay", "contact_points",
  "$@40", "max_reconnect_tries", "keyspace", "$@41", "consistency", "$@42",
  "serial_consistency", "$@43", "sanity_checks", "$@44",
  "sanity_checks_params", "sanity_checks_param", "lease_checks", "$@45",
  "mac_sources", "$@46", "mac_sources_list", "mac_sources_value",
  "duid_id", "string_id", "host_reservation_identifiers", "$@47",
  "host_reservation_identifiers_list", "host_reservation_identifier",
  "hw_address_id", "flex_id", "relay_supplied_options", "$@48",
  "dhcp_multi_threading", "$@49", "multi_threading_params",
  "multi_threading_param", "enable_multi_threading", "thread_pool_size",
  "packet_queue_size", "hooks_libraries", "$@50", "hooks_libraries_list",
  "not_empty_hooks_libraries_list", "hooks_library", "$@51",
  "sub_hooks_library", "$@52", "hooks_params", "hooks_param", "library",
  "$@53", "library-path", "$@54", "parameters", "$@55",
  "expired_leases_processing", "$@56", "expired_leases_params",
  "expired_leases_param", "reclaim_timer_wait_time",
  "flush_reclaimed_timer_wait_time", "hold_reclaimed_time",
  "max_reclaim_leases", "max_reclaim_time", "unwarned_reclaim_cycles",
  "subnet6_list", "$@57", "subnet6_list_content", "not_empty_subnet6_list",
  "subnet6", "$@58", "sub_subnet6", "$@59", "subnet6_params",
  "subnet6_param", "subnet", "$@60", "interface", "$@61", "interface_id",
  "$@62", "client_class", "$@63", "require_client_classes", "$@64",
  "reservation_mode", "$@65", "hr_mode", "id", "rapid_commit",
  "shared_networks", "$@66", "shared_networks_content",
  "shared_networks_list", "shared_network", "$@67",
  "shared_network_params", "shared_network_param", "option_def_list",
  "$@68", "sub_option_def_list", "$@69", "option_def_list_content",
  "not_empty_option_def_list", "option_def_entry", "$@70",
  "sub_option_def", "$@71", "option_def_params",
  "not_empty_option_def_params", "option_def_param", "option_def_name",
  "code", "option_def_code", "option_def_type", "$@72",
  "option_def_record_types", "$@73", "space", "$@74", "option_def_space",
  "option_def_encapsulate", "$@75", "option_def_array", "option_data_list",
  "$@76", "option_data_list_content", "not_empty_option_data_list",
  "option_data_entry", "$@77", "sub_option_data", "$@78",
  "option_data_params", "not_empty_option_data_params",
  "option_data_param", "option_data_name", "option_data_data", "$@79",
  "option_data_code", "option_data_space", "option_data_csv_format",
  "option_data_always_send", "pools_list", "$@80", "pools_list_content",
  "not_empty_pools_list", "pool_list_entry", "$@81", "sub_pool6", "$@82",
  "pool_params", "pool_param", "pool_entry", "$@83", "user_context",
  "$@84", "comment", "$@85", "pd_pools_list", "$@86",
  "pd_pools_list_content", "not_empty_pd_pools_list", "pd_pool_entry",
  "$@87", "sub_pd_pool", "$@88", "pd_pool_params", "pd_pool_param",
  "pd_prefix", "$@89", "pd_prefix_len", "excluded_prefix", "$@90",
  "excluded_prefix_len", "pd_delegated_len", "reservations", "$@91",
  "reservations_list", "not_empty_reservations_list", "reservation",
  "$@92", "sub_reservation", "$@93", "reservation_params",
  "not_empty_reservation_params", "reservation_param", "ip_addresses",
  "$@94", "prefixes", "$@95", "duid", "$@96", "hw_address", "$@97",
  "hostname", "$@98", "flex_id_value", "$@99",
  "reservation_client_classes", "$@100", "relay", "$@101", "relay_map",
  "ip_address", "$@102", "client_classes", "$@103", "client_classes_list",
  "client_class_entry", "$@104", "client_class_params",
  "not_empty_client_class_params", "client_class_param",
  "client_class_name", "client_class_test", "$@105", "only_if_required",
  "server_id", "$@106", "server_id_params", "server_id_param",
  "server_id_type", "$@107", "duid_type", "htype", "identifier", "$@108",
  "time", "enterprise_id", "dhcp4o6_port", "control_socket", "$@109",
  "control_socket_params", "control_socket_param",
  "notifications_control_socket_params",
  "notifications_control_socket_param", "socket_type", "$@110",
  "socket_name", "$@111", "on-boot-request-configuration-from-kea-netconf",
  "dhcp_queue_control", "$@112", "queue_control_params",
  "queue_control_param", "enable_queue", "queue_type", "$@113", "capacity",
  "arbitrary_map_entry", "$@114", "dhcp_ddns", "$@115", "sub_dhcp_ddns",
  "$@116", "dhcp_ddns_params", "dhcp_ddns_param", "enable_updates",
  "qualifying_suffix", "$@117", "server_ip", "$@118", "server_port",
  "sender_ip", "$@119", "sender_port", "max_queue_size", "ncr_protocol",
  "$@120", "ncr_protocol_value", "ncr_format", "$@121",
  "dep_override_no_update", "dep_override_client_update",
  "dep_replace_client_name", "$@122", "dep_generated_prefix", "$@123",
  "dep_hostname_char_set", "$@124", "dep_hostname_char_replacement",
  "$@125", "config_control", "$@126", "sub_config_control", "$@127",
  "config_control_params", "config_control_param", "config_databases",
  "$@128", "config_fetch_wait_time", "loggers", "$@129", "loggers_entries",
  "logger_entry", "$@130", "logger_params", "logger_param", "debuglevel",
  "severity", "$@131", "output_options_list", "$@132",
  "output_options_list_content", "output_entry", "$@133",
  "output_params_list", "output_params", "output", "$@134", "flush",
  "maxsize", "maxver", "pattern", "$@135", "allocation_type", "$@136",
  "config_database", "$@137", "configuration_type", "$@138", "instance_id",
  "$@139", "lawful-interception-parameters", "$@140", "master_database",
  "$@141", "max_address_utilization", "max_pd_space_utilization",
  "max_statement_tries", "notifications", "$@142",
  "option_set_description", "option_set_id", "policy-engine-parameters",
  "$@143", "privacy_history_size", "privacy_valid_lifetime", "protocol",
  "$@144", "ssl_cert", "$@145", YY_NULLPTR
  };

#if PARSER6_DEBUG
  const short
  Dhcp6Parser::yyrline_[] =
  {
       0,   304,   304,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   310,   310,   311,   311,   312,   312,
     313,   313,   314,   314,   315,   315,   316,   316,   317,   317,
     325,   326,   327,   328,   329,   330,   331,   334,   339,   339,
     350,   353,   354,   357,   361,   368,   368,   375,   376,   379,
     383,   390,   390,   397,   398,   401,   405,   416,   425,   425,
     440,   441,   445,   448,   448,   465,   465,   474,   475,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     491,   492,   493,   494,   495,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,   533,   534,   535,   536,   537,   538,   539,   540,
     541,   544,   544,   552,   557,   562,   567,   572,   577,   582,
     587,   592,   597,   602,   607,   612,   617,   622,   627,   627,
     635,   638,   641,   644,   647,   653,   653,   661,   661,   669,
     669,   677,   677,   685,   690,   695,   700,   700,   708,   708,
     719,   719,   728,   729,   732,   733,   734,   735,   736,   739,
     739,   749,   754,   754,   766,   766,   778,   778,   788,   789,
     792,   793,   796,   796,   806,   807,   810,   811,   812,   813,
     814,   815,   816,   817,   818,   819,   820,   821,   822,   823,
     824,   825,   826,   827,   828,   829,   830,   831,   832,   833,
     836,   836,   843,   844,   845,   846,   849,   849,   857,   857,
     865,   865,   873,   878,   878,   886,   891,   896,   901,   906,
     911,   916,   921,   926,   931,   931,   939,   944,   944,   952,
     952,   960,   960,   968,   968,   978,   979,   981,   983,   983,
    1001,  1001,  1011,  1012,  1015,  1016,  1019,  1024,  1029,  1029,
    1039,  1040,  1043,  1044,  1045,  1048,  1053,  1060,  1060,  1072,
    1072,  1084,  1085,  1088,  1089,  1090,  1091,  1092,  1093,  1096,
    1101,  1106,  1111,  1111,  1121,  1122,  1125,  1126,  1129,  1129,
    1140,  1140,  1151,  1152,  1153,  1156,  1157,  1158,  1159,  1160,
    1163,  1163,  1171,  1171,  1179,  1179,  1187,  1187,  1198,  1199,
    1202,  1203,  1204,  1205,  1206,  1207,  1210,  1215,  1220,  1225,
    1230,  1235,  1243,  1243,  1256,  1257,  1260,  1261,  1268,  1268,
    1294,  1294,  1305,  1306,  1310,  1311,  1312,  1313,  1314,  1315,
    1316,  1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,  1325,
    1326,  1327,  1328,  1329,  1330,  1331,  1332,  1333,  1334,  1335,
    1336,  1337,  1338,  1339,  1340,  1341,  1342,  1343,  1344,  1345,
    1346,  1349,  1349,  1357,  1357,  1365,  1365,  1373,  1373,  1381,
    1381,  1391,  1391,  1398,  1399,  1400,  1401,  1404,  1409,  1416,
    1416,  1427,  1428,  1432,  1433,  1436,  1436,  1444,  1445,  1448,
    1449,  1450,  1451,  1452,  1453,  1454,  1455,  1456,  1457,  1458,
    1459,  1460,  1461,  1462,  1463,  1464,  1465,  1466,  1467,  1468,
    1469,  1470,  1471,  1472,  1473,  1474,  1475,  1476,  1477,  1478,
    1479,  1480,  1487,  1487,  1500,  1500,  1509,  1510,  1513,  1514,
    1519,  1519,  1534,  1534,  1548,  1549,  1552,  1553,  1556,  1557,
    1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,  1568,  1570,
    1575,  1577,  1577,  1585,  1585,  1593,  1593,  1601,  1603,  1603,
    1611,  1620,  1620,  1632,  1633,  1638,  1639,  1644,  1644,  1656,
    1656,  1668,  1669,  1674,  1675,  1680,  1681,  1682,  1683,  1684,
    1685,  1686,  1687,  1688,  1691,  1693,  1693,  1701,  1703,  1705,
    1710,  1718,  1718,  1730,  1731,  1734,  1735,  1738,  1738,  1748,
    1748,  1758,  1759,  1762,  1763,  1764,  1765,  1766,  1767,  1768,
    1769,  1770,  1773,  1773,  1781,  1781,  1806,  1806,  1836,  1836,
    1848,  1849,  1852,  1853,  1856,  1856,  1868,  1868,  1880,  1881,
    1884,  1885,  1886,  1887,  1888,  1889,  1890,  1891,  1892,  1893,
    1894,  1895,  1896,  1899,  1899,  1907,  1912,  1912,  1920,  1925,
    1933,  1933,  1943,  1944,  1947,  1948,  1951,  1951,  1960,  1960,
    1969,  1970,  1973,  1974,  1978,  1979,  1980,  1981,  1982,  1983,
    1984,  1985,  1986,  1987,  1988,  1991,  1991,  2001,  2001,  2011,
    2011,  2019,  2019,  2027,  2027,  2035,  2035,  2043,  2043,  2056,
    2056,  2066,  2067,  2070,  2070,  2081,  2081,  2091,  2092,  2095,
    2095,  2105,  2106,  2109,  2110,  2113,  2114,  2115,  2116,  2117,
    2118,  2119,  2122,  2124,  2124,  2132,  2140,  2140,  2152,  2153,
    2156,  2157,  2158,  2159,  2160,  2161,  2162,  2163,  2164,  2167,
    2167,  2174,  2175,  2176,  2179,  2184,  2184,  2192,  2197,  2204,
    2211,  2211,  2221,  2222,  2225,  2226,  2227,  2228,  2229,  2232,
    2233,  2236,  2237,  2238,  2239,  2240,  2241,  2244,  2244,  2252,
    2252,  2260,  2267,  2267,  2279,  2280,  2283,  2284,  2285,  2286,
    2287,  2288,  2291,  2296,  2296,  2304,  2309,  2309,  2318,  2318,
    2330,  2330,  2340,  2341,  2344,  2345,  2346,  2347,  2348,  2349,
    2350,  2351,  2352,  2353,  2354,  2355,  2356,  2357,  2358,  2359,
    2360,  2361,  2364,  2369,  2369,  2377,  2377,  2385,  2390,  2390,
    2398,  2403,  2408,  2408,  2416,  2417,  2420,  2420,  2429,  2435,
    2441,  2441,  2449,  2449,  2458,  2458,  2467,  2467,  2478,  2478,
    2489,  2489,  2499,  2500,  2504,  2505,  2508,  2508,  2518,  2525,
    2525,  2537,  2538,  2542,  2542,  2550,  2551,  2554,  2555,  2556,
    2557,  2558,  2559,  2560,  2563,  2568,  2568,  2576,  2576,  2586,
    2587,  2590,  2590,  2598,  2599,  2602,  2603,  2604,  2605,  2606,
    2609,  2609,  2617,  2622,  2627,  2632,  2632,  2642,  2642,  2650,
    2650,  2660,  2660,  2668,  2668,  2677,  2677,  2684,  2684,  2694,
    2699,  2704,  2709,  2709,  2719,  2724,  2729,  2729,  2736,  2741,
    2746,  2746,  2754,  2754
  };

  // Print the state stack on the debug stream.
  void
  Dhcp6Parser::yystack_print_ ()
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
  Dhcp6Parser::yy_reduce_print_ (int yyrule)
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
#endif // PARSER6_DEBUG


#line 14 "dhcp6_parser.yy"
} } // isc::dhcp
#line 5911 "dhcp6_parser.cc"

#line 2762 "dhcp6_parser.yy"


void
isc::dhcp::Dhcp6Parser::error(const location_type& loc,
                              const std::string& what)
{
    ctx.error(loc, what);
}
