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
#define yylex   netconf_lex



#include "netconf_parser.h"


// Unqualified %code blocks.
#line 33 "netconf_parser.yy"

#include <netconf/parser_context.h>

#line 51 "netconf_parser.cc"


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
#if NETCONF_DEBUG

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

#else // !NETCONF_DEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !NETCONF_DEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 14 "netconf_parser.yy"
namespace isc { namespace netconf {
#line 143 "netconf_parser.cc"


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  NetconfParser::yytnamerr_ (const char *yystr)
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
  NetconfParser::NetconfParser (isc::netconf::ParserContext& ctx_yyarg)
#if NETCONF_DEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      ctx (ctx_yyarg)
  {}

  NetconfParser::~NetconfParser ()
  {}

  NetconfParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  NetconfParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  NetconfParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  NetconfParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  NetconfParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  NetconfParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  NetconfParser::symbol_number_type
  NetconfParser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  NetconfParser::stack_symbol_type::stack_symbol_type ()
  {}

  NetconfParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 59: // value
      case 62: // map_value
      case 128: // socket_type_value
        value.YY_MOVE_OR_COPY< ElementPtr > (YY_MOVE (that.value));
        break;

      case 50: // "boolean"
        value.YY_MOVE_OR_COPY< bool > (YY_MOVE (that.value));
        break;

      case 49: // "floating point"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case 48: // "integer"
        value.YY_MOVE_OR_COPY< int64_t > (YY_MOVE (that.value));
        break;

      case 47: // "constant string"
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

  NetconfParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 59: // value
      case 62: // map_value
      case 128: // socket_type_value
        value.move< ElementPtr > (YY_MOVE (that.value));
        break;

      case 50: // "boolean"
        value.move< bool > (YY_MOVE (that.value));
        break;

      case 49: // "floating point"
        value.move< double > (YY_MOVE (that.value));
        break;

      case 48: // "integer"
        value.move< int64_t > (YY_MOVE (that.value));
        break;

      case 47: // "constant string"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  NetconfParser::stack_symbol_type&
  NetconfParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 59: // value
      case 62: // map_value
      case 128: // socket_type_value
        value.copy< ElementPtr > (that.value);
        break;

      case 50: // "boolean"
        value.copy< bool > (that.value);
        break;

      case 49: // "floating point"
        value.copy< double > (that.value);
        break;

      case 48: // "integer"
        value.copy< int64_t > (that.value);
        break;

      case 47: // "constant string"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  NetconfParser::stack_symbol_type&
  NetconfParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 59: // value
      case 62: // map_value
      case 128: // socket_type_value
        value.move< ElementPtr > (that.value);
        break;

      case 50: // "boolean"
        value.move< bool > (that.value);
        break;

      case 49: // "floating point"
        value.move< double > (that.value);
        break;

      case 48: // "integer"
        value.move< int64_t > (that.value);
        break;

      case 47: // "constant string"
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
  NetconfParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if NETCONF_DEBUG
  template <typename Base>
  void
  NetconfParser::yy_print_ (std::ostream& yyo,
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
      case 47: // "constant string"
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < std::string > (); }
#line 427 "netconf_parser.cc"
        break;

      case 48: // "integer"
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < int64_t > (); }
#line 433 "netconf_parser.cc"
        break;

      case 49: // "floating point"
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < double > (); }
#line 439 "netconf_parser.cc"
        break;

      case 50: // "boolean"
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < bool > (); }
#line 445 "netconf_parser.cc"
        break;

      case 59: // value
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 451 "netconf_parser.cc"
        break;

      case 62: // map_value
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 457 "netconf_parser.cc"
        break;

      case 128: // socket_type_value
#line 108 "netconf_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 463 "netconf_parser.cc"
        break;

      default:
        break;
    }
    yyo << ')';
  }
#endif

  void
  NetconfParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  NetconfParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  NetconfParser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if NETCONF_DEBUG
  std::ostream&
  NetconfParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  NetconfParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  NetconfParser::debug_level_type
  NetconfParser::debug_level () const
  {
    return yydebug_;
  }

  void
  NetconfParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // NETCONF_DEBUG

  NetconfParser::state_type
  NetconfParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  NetconfParser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  NetconfParser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  NetconfParser::operator() ()
  {
    return parse ();
  }

  int
  NetconfParser::parse ()
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
      case 59: // value
      case 62: // map_value
      case 128: // socket_type_value
        yylhs.value.emplace< ElementPtr > ();
        break;

      case 50: // "boolean"
        yylhs.value.emplace< bool > ();
        break;

      case 49: // "floating point"
        yylhs.value.emplace< double > ();
        break;

      case 48: // "integer"
        yylhs.value.emplace< int64_t > ();
        break;

      case 47: // "constant string"
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
#line 119 "netconf_parser.yy"
                       { ctx.ctx_ = ctx.NO_KEYWORDS; }
#line 726 "netconf_parser.cc"
    break;

  case 4:
#line 120 "netconf_parser.yy"
                         { ctx.ctx_ = ctx.CONFIG; }
#line 732 "netconf_parser.cc"
    break;

  case 6:
#line 121 "netconf_parser.yy"
                         { ctx.ctx_ = ctx.NETCONF; }
#line 738 "netconf_parser.cc"
    break;

  case 8:
#line 129 "netconf_parser.yy"
                            {
    // Parse the Netconf map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 748 "netconf_parser.cc"
    break;

  case 9:
#line 133 "netconf_parser.yy"
                               {
    // parsing completed
}
#line 756 "netconf_parser.cc"
    break;

  case 10:
#line 140 "netconf_parser.yy"
            {
    // Push back the JSON value on the stack
    ctx.stack_.push_back(yystack_[0].value.as < ElementPtr > ());
}
#line 765 "netconf_parser.cc"
    break;

  case 11:
#line 146 "netconf_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location))); }
#line 771 "netconf_parser.cc"
    break;

  case 12:
#line 147 "netconf_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location))); }
#line 777 "netconf_parser.cc"
    break;

  case 13:
#line 148 "netconf_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location))); }
#line 783 "netconf_parser.cc"
    break;

  case 14:
#line 149 "netconf_parser.yy"
              { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location))); }
#line 789 "netconf_parser.cc"
    break;

  case 15:
#line 150 "netconf_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new NullElement(ctx.loc2pos(yystack_[0].location))); }
#line 795 "netconf_parser.cc"
    break;

  case 16:
#line 151 "netconf_parser.yy"
           { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 801 "netconf_parser.cc"
    break;

  case 17:
#line 152 "netconf_parser.yy"
                    { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 807 "netconf_parser.cc"
    break;

  case 18:
#line 156 "netconf_parser.yy"
                    {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 818 "netconf_parser.cc"
    break;

  case 19:
#line 161 "netconf_parser.yy"
                             {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
}
#line 828 "netconf_parser.cc"
    break;

  case 20:
#line 167 "netconf_parser.yy"
               { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 834 "netconf_parser.cc"
    break;

  case 23:
#line 181 "netconf_parser.yy"
                                  {
                  // map containing a single entry
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 843 "netconf_parser.cc"
    break;

  case 24:
#line 185 "netconf_parser.yy"
                                                      {
                  // map consisting of a shorter map followed by
                  // comma and string:value
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 853 "netconf_parser.cc"
    break;

  case 25:
#line 192 "netconf_parser.yy"
                              {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(l);
}
#line 862 "netconf_parser.cc"
    break;

  case 26:
#line 195 "netconf_parser.yy"
                               {
}
#line 869 "netconf_parser.cc"
    break;

  case 29:
#line 202 "netconf_parser.yy"
                      {
                  // List consisting of a single element.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 878 "netconf_parser.cc"
    break;

  case 30:
#line 206 "netconf_parser.yy"
                                           {
                  // List ending with , and a value.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 887 "netconf_parser.cc"
    break;

  case 31:
#line 219 "netconf_parser.yy"
                                {
    const std::string& where = ctx.contextName();
    const std::string& keyword = yystack_[1].value.as < std::string > ();
    error(yystack_[1].location,
          "got unexpected keyword \"" + keyword + "\" in " + where + " map.");
}
#line 898 "netconf_parser.cc"
    break;

  case 32:
#line 227 "netconf_parser.yy"
                                   {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 909 "netconf_parser.cc"
    break;

  case 33:
#line 232 "netconf_parser.yy"
                                {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
}
#line 919 "netconf_parser.cc"
    break;

  case 37:
#line 248 "netconf_parser.yy"
                        {

    // Let's create a MapElement that will represent it, add it to the
    // top level map (that's already on the stack) and put the new map
    // on the stack as well, so child elements will be able to add
    // themselves to it.

    // Prevent against duplicate.
    ctx.unique("Netconf", ctx.loc2pos(yystack_[0].location));
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("Netconf", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.NETCONF);
}
#line 938 "netconf_parser.cc"
    break;

  case 38:
#line 261 "netconf_parser.yy"
                                                    {
    // Ok, we're done with parsing Netconf. Let's take the map
    // off the stack.
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 949 "netconf_parser.cc"
    break;

  case 49:
#line 286 "netconf_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 957 "netconf_parser.cc"
    break;

  case 50:
#line 288 "netconf_parser.yy"
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
#line 984 "netconf_parser.cc"
    break;

  case 51:
#line 311 "netconf_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 992 "netconf_parser.cc"
    break;

  case 52:
#line 313 "netconf_parser.yy"
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
#line 1021 "netconf_parser.cc"
    break;

  case 53:
#line 339 "netconf_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hooks-libraries", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOOKS_LIBRARIES);
}
#line 1032 "netconf_parser.cc"
    break;

  case 54:
#line 344 "netconf_parser.yy"
                                                             {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1041 "netconf_parser.cc"
    break;

  case 59:
#line 357 "netconf_parser.yy"
                              {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1051 "netconf_parser.cc"
    break;

  case 60:
#line 361 "netconf_parser.yy"
                              {
    ctx.stack_.pop_back();
}
#line 1059 "netconf_parser.cc"
    break;

  case 66:
#line 374 "netconf_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1067 "netconf_parser.cc"
    break;

  case 67:
#line 376 "netconf_parser.yy"
               {
    ElementPtr lib(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("library", lib);
    ctx.leave();
}
#line 1077 "netconf_parser.cc"
    break;

  case 68:
#line 382 "netconf_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1085 "netconf_parser.cc"
    break;

  case 69:
#line 384 "netconf_parser.yy"
                  {
    ctx.stack_.back()->set("parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1094 "netconf_parser.cc"
    break;

  case 70:
#line 392 "netconf_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("configuration", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONFIGURATION);
}
#line 1105 "netconf_parser.cc"
    break;

  case 71:
#line 397 "netconf_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1114 "netconf_parser.cc"
    break;

  case 80:
#line 414 "netconf_parser.yy"
                               {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("control-socket", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKET);
}
#line 1125 "netconf_parser.cc"
    break;

  case 81:
#line 419 "netconf_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1134 "netconf_parser.cc"
    break;

  case 90:
#line 436 "netconf_parser.yy"
                            {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1144 "netconf_parser.cc"
    break;

  case 91:
#line 440 "netconf_parser.yy"
                              {
    ctx.stack_.pop_back();
}
#line 1152 "netconf_parser.cc"
    break;

  case 102:
#line 458 "netconf_parser.yy"
             {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1160 "netconf_parser.cc"
    break;

  case 103:
#line 460 "netconf_parser.yy"
               {
    ElementPtr model(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("model", model);
    ctx.leave();
}
#line 1170 "netconf_parser.cc"
    break;

  case 104:
#line 466 "netconf_parser.yy"
               {
    ElementPtr m(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("models", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.MODELS);
}
#line 1181 "netconf_parser.cc"
    break;

  case 105:
#line 471 "netconf_parser.yy"
                                                       {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1190 "netconf_parser.cc"
    break;

  case 108:
#line 480 "netconf_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("notifications", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.NOTIFICATIONS);
}
#line 1201 "netconf_parser.cc"
    break;

  case 109:
#line 485 "netconf_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1210 "netconf_parser.cc"
    break;

  case 115:
#line 499 "netconf_parser.yy"
                         {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("operational", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.OPERATIONAL);
}
#line 1221 "netconf_parser.cc"
    break;

  case 116:
#line 504 "netconf_parser.yy"
                                                       {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1230 "netconf_parser.cc"
    break;

  case 122:
#line 518 "netconf_parser.yy"
         {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("rpc", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.RPC);
}
#line 1241 "netconf_parser.cc"
    break;

  case 123:
#line 523 "netconf_parser.yy"
                                               {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1250 "netconf_parser.cc"
    break;

  case 129:
#line 538 "netconf_parser.yy"
                                                           {
    ElementPtr flag(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("on-boot-update-socket", flag);
}
#line 1259 "netconf_parser.cc"
    break;

  case 130:
#line 543 "netconf_parser.yy"
                                                             {
    ElementPtr flag(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("on-boot-update-sysrepo", flag);
}
#line 1268 "netconf_parser.cc"
    break;

  case 131:
#line 548 "netconf_parser.yy"
                         {
    ctx.enter(ctx.SOCKET_TYPE);
}
#line 1276 "netconf_parser.cc"
    break;

  case 132:
#line 550 "netconf_parser.yy"
                          {
    ctx.stack_.back()->set("socket-type", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1285 "netconf_parser.cc"
    break;

  case 133:
#line 555 "netconf_parser.yy"
                         { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("unix", ctx.loc2pos(yystack_[0].location))); }
#line 1291 "netconf_parser.cc"
    break;

  case 134:
#line 556 "netconf_parser.yy"
                                    { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("kea-config-tool", ctx.loc2pos(yystack_[0].location))); }
#line 1297 "netconf_parser.cc"
    break;

  case 135:
#line 557 "netconf_parser.yy"
                         { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("http", ctx.loc2pos(yystack_[0].location))); }
#line 1303 "netconf_parser.cc"
    break;

  case 136:
#line 558 "netconf_parser.yy"
                           { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("stdout", ctx.loc2pos(yystack_[0].location))); }
#line 1309 "netconf_parser.cc"
    break;

  case 137:
#line 561 "netconf_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1317 "netconf_parser.cc"
    break;

  case 138:
#line 563 "netconf_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-name", name);
    ctx.leave();
}
#line 1327 "netconf_parser.cc"
    break;

  case 139:
#line 569 "netconf_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1335 "netconf_parser.cc"
    break;

  case 140:
#line 571 "netconf_parser.yy"
               {
    ElementPtr url(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-url", url);
    ctx.leave();
}
#line 1345 "netconf_parser.cc"
    break;

  case 141:
#line 577 "netconf_parser.yy"
                                    {
     ElementPtr flag(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
     ctx.stack_.back()->set("subscribe", flag);
 }
#line 1354 "netconf_parser.cc"
    break;

  case 142:
#line 582 "netconf_parser.yy"
                                  {
     ElementPtr flag(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
     ctx.stack_.back()->set("validate", flag);
 }
#line 1363 "netconf_parser.cc"
    break;

  case 143:
#line 591 "netconf_parser.yy"
                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("loggers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.LOGGERS);
}
#line 1374 "netconf_parser.cc"
    break;

  case 144:
#line 596 "netconf_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1383 "netconf_parser.cc"
    break;

  case 147:
#line 608 "netconf_parser.yy"
                             {
    ElementPtr l(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(l);
    ctx.stack_.push_back(l);
}
#line 1393 "netconf_parser.cc"
    break;

  case 148:
#line 612 "netconf_parser.yy"
                               {
    ctx.stack_.pop_back();
}
#line 1401 "netconf_parser.cc"
    break;

  case 158:
#line 629 "netconf_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1409 "netconf_parser.cc"
    break;

  case 159:
#line 631 "netconf_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("name", name);
    ctx.leave();
}
#line 1419 "netconf_parser.cc"
    break;

  case 160:
#line 637 "netconf_parser.yy"
                                     {
    ElementPtr dl(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("debuglevel", dl);
}
#line 1428 "netconf_parser.cc"
    break;

  case 161:
#line 642 "netconf_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1436 "netconf_parser.cc"
    break;

  case 162:
#line 644 "netconf_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("severity", sev);
    ctx.leave();
}
#line 1446 "netconf_parser.cc"
    break;

  case 163:
#line 650 "netconf_parser.yy"
                                    {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output_options", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OUTPUT_OPTIONS);
}
#line 1457 "netconf_parser.cc"
    break;

  case 164:
#line 655 "netconf_parser.yy"
                                                                    {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1466 "netconf_parser.cc"
    break;

  case 167:
#line 664 "netconf_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1476 "netconf_parser.cc"
    break;

  case 168:
#line 668 "netconf_parser.yy"
                                    {
    ctx.stack_.pop_back();
}
#line 1484 "netconf_parser.cc"
    break;

  case 176:
#line 683 "netconf_parser.yy"
               {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1492 "netconf_parser.cc"
    break;

  case 177:
#line 685 "netconf_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output", sev);
    ctx.leave();
}
#line 1502 "netconf_parser.cc"
    break;

  case 178:
#line 691 "netconf_parser.yy"
                           {
    ElementPtr flush(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flush", flush);
}
#line 1511 "netconf_parser.cc"
    break;

  case 179:
#line 696 "netconf_parser.yy"
                               {
    ElementPtr maxsize(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxsize", maxsize);
}
#line 1520 "netconf_parser.cc"
    break;

  case 180:
#line 701 "netconf_parser.yy"
                             {
    ElementPtr maxver(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxver", maxver);
}
#line 1529 "netconf_parser.cc"
    break;

  case 181:
#line 706 "netconf_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1537 "netconf_parser.cc"
    break;

  case 182:
#line 708 "netconf_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pattern", sev);
    ctx.leave();
}
#line 1547 "netconf_parser.cc"
    break;


#line 1551 "netconf_parser.cc"

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
  NetconfParser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  NetconfParser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
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


  const short NetconfParser::yypact_ninf_ = -173;

  const signed char NetconfParser::yytable_ninf_ = -1;

  const short
  NetconfParser::yypact_[] =
  {
      93,  -173,  -173,  -173,    14,    23,    17,    34,  -173,  -173,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,
    -173,  -173,  -173,  -173,    23,    -4,    50,    11,  -173,    62,
      72,    94,   101,   118,  -173,    48,  -173,  -173,  -173,  -173,
    -173,  -173,  -173,   121,  -173,   115,   129,  -173,  -173,  -173,
    -173,  -173,  -173,  -173,    23,    23,  -173,    86,   131,    50,
    -173,   136,   137,   138,   139,   140,  -173,  -173,    11,  -173,
    -173,   141,   142,  -173,   143,    99,   146,   147,   148,  -173,
      23,    11,  -173,  -173,  -173,   149,   153,   154,  -173,   155,
    -173,  -173,    12,  -173,   156,   144,  -173,  -173,    31,  -173,
    -173,    27,   149,  -173,   -16,  -173,   153,    65,   154,  -173,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,    76,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,    83,
    -173,  -173,  -173,  -173,  -173,  -173,   150,  -173,  -173,  -173,
    -173,    84,  -173,  -173,  -173,  -173,  -173,  -173,   151,   160,
     161,   162,   163,    27,  -173,   164,   165,    63,  -173,   166,
     167,   100,   168,    65,  -173,   169,   127,   170,   171,   172,
    -173,   133,   143,  -173,   134,   177,  -173,   145,  -173,    69,
    -173,    19,    19,    19,  -173,  -173,  -173,   176,  -173,  -173,
     180,   184,   185,   186,  -173,   103,  -173,  -173,  -173,  -173,
    -173,  -173,  -173,  -173,   105,  -173,  -173,  -173,  -173,   107,
    -173,  -173,  -173,  -173,   119,  -173,  -173,  -173,    82,  -173,
     187,   123,   125,   152,   157,    69,  -173,    19,  -173,    19,
    -173,    19,  -173,    77,   176,  -173,   188,  -173,  -173,  -173,
    -173,  -173,  -173,  -173,  -173,  -173,   189,   190,   192,  -173,
     126,  -173,  -173,  -173,  -173,  -173,  -173,  -173,     1,   193,
     158,   173,   174,   194,    77,  -173,  -173,  -173,  -173,  -173,
    -173,  -173,   128,  -173,  -173,  -173,  -173,   159,  -173,  -173,
    -173,   178,  -173,   195,   196,   197,     1,  -173,  -173,  -173,
     179,    78,   181,  -173,  -173,  -173,  -173,  -173,  -173,  -173,
    -173
  };

  const unsigned char
  NetconfParser::yydefact_[] =
  {
       0,     2,     4,     6,     0,     0,     0,     0,     1,    25,
      18,    15,    14,    11,    12,    13,     3,    10,    16,    17,
      32,     5,     8,     7,    27,    21,     0,    39,    29,     0,
      28,     0,     0,    22,    37,     0,    34,    36,    49,    51,
     104,    53,   143,     0,    48,     0,    40,    41,    46,    47,
      44,    43,    45,    26,     0,     0,    19,     0,     0,     0,
      33,     0,     0,     0,     0,     0,    31,     9,     0,    30,
      23,     0,     0,    35,     0,     0,     0,     0,     0,    42,
       0,    39,    20,    50,    52,     0,    55,     0,    24,     0,
      90,   106,     0,    59,     0,    56,    57,   147,     0,   145,
      38,     0,     0,   105,     0,    54,     0,     0,     0,   144,
      70,   102,   108,   115,   122,   101,   100,    94,    95,     0,
      92,    96,    97,    98,    99,   107,    66,    68,    63,     0,
      61,    64,    65,    58,   158,   163,     0,   161,   157,   155,
     156,     0,   149,   151,   153,   154,   152,   146,     0,     0,
       0,     0,     0,     0,    91,     0,     0,     0,    60,     0,
       0,     0,     0,     0,   148,     0,     0,     0,     0,     0,
      93,     0,     0,    62,     0,     0,   160,     0,   150,     0,
     103,     0,     0,     0,    67,    69,   159,     0,   162,    80,
       0,     0,     0,     0,    79,     0,    72,    74,    75,    76,
      77,    78,   114,   112,     0,   110,   113,   121,   119,     0,
     117,   120,   128,   126,     0,   124,   127,   167,     0,   165,
       0,     0,     0,     0,     0,     0,    71,     0,   109,     0,
     116,     0,   123,     0,     0,   164,     0,   129,   130,   141,
     142,    73,   111,   118,   125,   176,     0,     0,     0,   181,
       0,   169,   171,   172,   173,   174,   175,   166,     0,     0,
       0,     0,     0,     0,     0,   168,   137,   131,   139,    89,
      87,    88,     0,    82,    84,    85,    86,     0,   178,   179,
     180,     0,   170,     0,     0,     0,     0,    81,   177,   182,
       0,     0,     0,    83,   138,   135,   134,   136,   133,   132,
     140
  };

  const short
  NetconfParser::yypgoto_[] =
  {
    -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,   -19,   -70,
    -173,    32,  -173,  -173,  -173,  -173,  -173,  -173,  -101,  -173,
    -173,  -173,   175,  -173,  -173,   122,  -173,   182,  -100,  -173,
     -99,  -173,  -173,  -173,  -173,  -173,   104,  -173,  -173,    52,
    -173,  -173,  -173,  -173,  -173,  -173,  -173,   -20,  -172,  -173,
    -173,   -75,   110,  -173,  -173,    60,  -173,  -173,  -173,  -173,
    -173,  -173,  -173,  -173,   -13,  -173,  -173,  -173,   -14,  -173,
    -173,  -173,   -15,  -173,  -173,  -173,  -173,  -173,  -173,  -173,
    -173,  -173,  -162,  -173,  -173,  -173,  -173,   109,  -173,  -173,
      55,  -173,  -173,  -173,  -173,  -173,  -173,  -173,  -173,   -11,
    -173,  -173,   -45,  -173,  -173,  -173,  -173,  -173,  -173,  -173
  };

  const short
  NetconfParser::yydefgoto_[] =
  {
      -1,     4,     5,     6,     7,    23,    27,    16,    17,    18,
      25,    83,    32,    33,    19,    24,    29,    30,    44,    21,
      26,    35,    36,    37,    58,    45,    46,    47,    48,    61,
      49,    62,    50,    64,    94,    95,    96,   104,   129,   130,
     131,   155,   132,   156,   118,   148,   195,   196,   197,   220,
     272,   273,    91,   101,   119,   120,   121,   149,    51,    63,
      92,   122,   150,   204,   205,   123,   151,   209,   210,   124,
     152,   214,   215,   198,   199,   274,   284,   299,   275,   283,
     276,   285,   200,   201,    52,    65,    98,    99,   107,   141,
     142,   143,   159,   144,   145,   162,   146,   160,   218,   219,
     233,   250,   251,   252,   259,   253,   254,   255,   256,   263
  };

  const short
  NetconfParser::yytable_[] =
  {
     115,   116,   117,   128,    82,    28,   138,   139,   140,   203,
     208,   213,    38,    39,     8,   102,   126,   127,   103,   206,
     211,   216,    38,    39,    20,   266,   267,   268,     9,    40,
      10,    43,    11,   189,   108,    69,    70,   109,    38,    39,
     110,    22,    41,    31,   111,    42,   112,   192,    43,   113,
     114,    59,   115,   116,   117,   203,    60,   208,    43,   213,
      34,    88,   138,   139,   140,   206,    43,   211,    53,   216,
      12,    13,    14,    15,    43,    54,    38,    39,   194,   153,
     202,   207,   212,   189,   154,   234,   157,   163,   235,   190,
     191,   158,   164,   295,   296,   126,   127,   192,    55,   193,
     134,   135,    82,   136,   137,   297,   225,   298,   227,    56,
     229,   226,    43,   228,   245,   230,    43,   246,   247,   248,
     249,    57,   231,    67,   194,    66,   202,   232,   207,   264,
     212,   286,    68,    71,   265,    72,   287,     1,     2,     3,
      74,    75,    76,    77,    78,    80,    84,   106,   176,    81,
      10,    85,    86,    87,   161,   165,    90,   269,   270,   271,
      93,    97,   105,   100,   166,   167,   168,   169,   171,   172,
     174,   175,   177,   237,   180,   238,   179,   181,   182,   183,
     184,   186,   187,   217,   221,   269,   270,   271,   222,   223,
     224,   236,   188,   260,   261,   258,   262,   277,   281,   290,
     291,   292,   239,    89,   185,   241,   288,   240,   278,   173,
     133,   293,   125,   170,   242,   243,   244,   147,   178,   282,
       0,   279,   280,   257,     0,   289,   294,     0,   300,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      79
  };

  const short
  NetconfParser::yycheck_[] =
  {
     101,   101,   101,   104,    74,    24,   107,   107,   107,   181,
     182,   183,    11,    12,     0,     3,    32,    33,     6,   181,
     182,   183,    11,    12,     7,    24,    25,    26,     5,    18,
       7,    47,     9,    14,     3,    54,    55,     6,    11,    12,
      13,     7,    31,    47,    17,    34,    19,    28,    47,    22,
      23,     3,   153,   153,   153,   227,     8,   229,    47,   231,
      10,    80,   163,   163,   163,   227,    47,   229,     6,   231,
      47,    48,    49,    50,    47,     3,    11,    12,   179,     3,
     181,   182,   183,    14,     8,     3,     3,     3,     6,    20,
      21,     8,     8,    15,    16,    32,    33,    28,     4,    30,
      35,    36,   172,    38,    39,    27,     3,    29,     3,     8,
       3,     8,    47,     8,    37,     8,    47,    40,    41,    42,
      43,     3,     3,     8,   225,     4,   227,     8,   229,     3,
     231,     3,     3,    47,     8,     4,     8,    44,    45,    46,
       4,     4,     4,     4,     4,     4,    47,     3,    48,     7,
       7,     5,     5,     5,     4,     4,     7,   258,   258,   258,
       7,     7,     6,     8,     4,     4,     4,     4,     4,     4,
       4,     4,     4,    50,    47,    50,     7,     7,     7,     7,
      47,    47,     5,     7,     4,   286,   286,   286,     4,     4,
       4,     4,    47,     4,     4,     7,     4,     4,     4,     4,
       4,     4,    50,    81,   172,   225,    47,    50,    50,   157,
     106,   286,   102,   153,   227,   229,   231,   108,   163,   264,
      -1,    48,    48,   234,    -1,    47,    47,    -1,    47,    -1,
      -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68
  };

  const unsigned char
  NetconfParser::yystos_[] =
  {
       0,    44,    45,    46,    52,    53,    54,    55,     0,     5,
       7,     9,    47,    48,    49,    50,    58,    59,    60,    65,
       7,    70,     7,    56,    66,    61,    71,    57,    59,    67,
      68,    47,    63,    64,    10,    72,    73,    74,    11,    12,
      18,    31,    34,    47,    69,    76,    77,    78,    79,    81,
      83,   109,   135,     6,     3,     4,     8,     3,    75,     3,
       8,    80,    82,   110,    84,   136,     4,     8,     3,    59,
      59,    47,     4,    73,     4,     4,     4,     4,     4,    78,
       4,     7,    60,    62,    47,     5,     5,     5,    59,    76,
       7,   103,   111,     7,    85,    86,    87,     7,   137,   138,
       8,   104,     3,     6,    88,     6,     3,   139,     3,     6,
      13,    17,    19,    22,    23,    69,    79,    81,    95,   105,
     106,   107,   112,   116,   120,   103,    32,    33,    69,    89,
      90,    91,    93,    87,    35,    36,    38,    39,    69,    79,
      81,   140,   141,   142,   144,   145,   147,   138,    96,   108,
     113,   117,   121,     3,     8,    92,    94,     3,     8,   143,
     148,     4,   146,     3,     8,     4,     4,     4,     4,     4,
     106,     4,     4,    90,     4,     4,    48,     4,   141,     7,
      47,     7,     7,     7,    47,    62,    47,     5,    47,    14,
      20,    21,    28,    30,    69,    97,    98,    99,   124,   125,
     133,   134,    69,    99,   114,   115,   133,    69,    99,   118,
     119,   133,    69,    99,   122,   123,   133,     7,   149,   150,
     100,     4,     4,     4,     4,     3,     8,     3,     8,     3,
       8,     3,     8,   151,     3,     6,     4,    50,    50,    50,
      50,    98,   115,   119,   123,    37,    40,    41,    42,    43,
     152,   153,   154,   156,   157,   158,   159,   150,     7,   155,
       4,     4,     4,   160,     3,     8,    24,    25,    26,    69,
      79,    81,   101,   102,   126,   129,   131,     4,    50,    48,
      48,     4,   153,   130,   127,   132,     3,     8,    47,    47,
       4,     4,     4,   102,    47,    15,    16,    27,    29,   128,
      47
  };

  const unsigned char
  NetconfParser::yyr1_[] =
  {
       0,    51,    53,    52,    54,    52,    55,    52,    57,    56,
      58,    59,    59,    59,    59,    59,    59,    59,    61,    60,
      62,    63,    63,    64,    64,    66,    65,    67,    67,    68,
      68,    69,    71,    70,    72,    72,    73,    75,    74,    76,
      76,    77,    77,    78,    78,    78,    78,    78,    78,    80,
      79,    82,    81,    84,    83,    85,    85,    86,    86,    88,
      87,    89,    89,    89,    90,    90,    92,    91,    94,    93,
      96,    95,    97,    97,    98,    98,    98,    98,    98,    98,
     100,    99,   101,   101,   102,   102,   102,   102,   102,   102,
     104,   103,   105,   105,   106,   106,   106,   106,   106,   106,
     106,   106,   108,   107,   110,   109,   111,   111,   113,   112,
     114,   114,   115,   115,   115,   117,   116,   118,   118,   119,
     119,   119,   121,   120,   122,   122,   123,   123,   123,   124,
     125,   127,   126,   128,   128,   128,   128,   130,   129,   132,
     131,   133,   134,   136,   135,   137,   137,   139,   138,   140,
     140,   141,   141,   141,   141,   141,   141,   141,   143,   142,
     144,   146,   145,   148,   147,   149,   149,   151,   150,   152,
     152,   153,   153,   153,   153,   153,   155,   154,   156,   157,
     158,   160,   159
  };

  const signed char
  NetconfParser::yyr2_[] =
  {
       0,     2,     0,     3,     0,     3,     0,     3,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     4,
       1,     0,     1,     3,     5,     0,     4,     0,     1,     1,
       3,     2,     0,     4,     1,     3,     1,     0,     6,     0,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     0,
       4,     0,     4,     0,     6,     0,     1,     1,     3,     0,
       4,     1,     3,     1,     1,     1,     0,     4,     0,     4,
       0,     6,     1,     3,     1,     1,     1,     1,     1,     1,
       0,     6,     1,     3,     1,     1,     1,     1,     1,     1,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     0,     6,     1,     3,     0,     6,
       1,     3,     1,     1,     1,     0,     6,     1,     3,     1,
       1,     1,     0,     6,     1,     3,     1,     1,     1,     3,
       3,     0,     4,     1,     1,     1,     1,     0,     4,     0,
       4,     3,     3,     0,     6,     1,     3,     0,     4,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     0,     4,
       3,     0,     4,     0,     6,     1,     3,     0,     4,     1,
       3,     1,     1,     1,     1,     1,     0,     4,     3,     3,
       3,     0,     4
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const NetconfParser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\",\"", "\":\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"null\"", "\"Netconf\"", "\"user-context\"",
  "\"comment\"", "\"configuration\"", "\"control-socket\"", "\"http\"",
  "\"kea-config-tool\"", "\"model\"", "\"models\"", "\"notifications\"",
  "\"on-boot-update-socket\"", "\"on-boot-update-sysrepo\"",
  "\"operational\"", "\"rpc\"", "\"socket-name\"", "\"socket-type\"",
  "\"socket-url\"", "\"stdout\"", "\"subscribe\"", "\"unix\"",
  "\"validate\"", "\"hooks-libraries\"", "\"library\"", "\"parameters\"",
  "\"loggers\"", "\"name\"", "\"output_options\"", "\"output\"",
  "\"debuglevel\"", "\"severity\"", "\"flush\"", "\"maxsize\"",
  "\"maxver\"", "\"pattern\"", "START_JSON", "START_NETCONF",
  "START_SUB_NETCONF", "\"constant string\"", "\"integer\"",
  "\"floating point\"", "\"boolean\"", "$accept", "start", "$@1", "$@2",
  "$@3", "sub_netconf", "$@4", "json", "value", "map", "$@5", "map_value",
  "map_content", "not_empty_map", "list_generic", "$@6", "list_content",
  "not_empty_list", "unknown_map_entry", "netconf_syntax_map", "$@7",
  "global_objects", "global_object", "netconf_object", "$@8",
  "global_params", "not_empty_global_params", "global_param",
  "user_context", "$@9", "comment", "$@10", "hooks_libraries", "$@11",
  "hooks_libraries_list", "not_empty_hooks_libraries_list",
  "hooks_library", "$@12", "hooks_params", "hooks_param", "library",
  "$@13", "parameters", "$@14", "configuration", "$@15",
  "configuration_keys", "configuration_key", "control_socket", "$@16",
  "control_socket_params", "control_socket_param", "model_entry", "$@17",
  "model_params", "model_param", "model", "$@18", "models", "$@19",
  "models_entries", "notifications", "$@20", "notifications_keys",
  "notifications_key", "operational", "$@21", "operational_keys",
  "operational_key", "rpc", "$@22", "rpc_keys", "rpc_key",
  "on_boot_update_socket", "on_boot_update_sysrepo", "socket_type", "$@23",
  "socket_type_value", "socket_name", "$@24", "socket_url", "$@25",
  "subscribe", "validate", "loggers", "$@26", "loggers_entries",
  "logger_entry", "$@27", "logger_params", "logger_param", "name", "$@28",
  "debuglevel", "severity", "$@29", "output_options_list", "$@30",
  "output_options_list_content", "output_entry", "$@31",
  "output_params_list", "output_params", "output", "$@32", "flush",
  "maxsize", "maxver", "pattern", "$@33", YY_NULLPTR
  };

#if NETCONF_DEBUG
  const short
  NetconfParser::yyrline_[] =
  {
       0,   119,   119,   119,   120,   120,   121,   121,   129,   129,
     140,   146,   147,   148,   149,   150,   151,   152,   156,   156,
     167,   172,   173,   181,   185,   192,   192,   198,   199,   202,
     206,   219,   227,   227,   239,   240,   244,   248,   248,   268,
     269,   272,   273,   278,   279,   280,   281,   282,   283,   286,
     286,   311,   311,   339,   339,   349,   350,   353,   354,   357,
     357,   365,   366,   367,   370,   371,   374,   374,   382,   382,
     392,   392,   402,   403,   406,   407,   408,   409,   410,   411,
     414,   414,   424,   425,   428,   429,   430,   431,   432,   433,
     436,   436,   444,   445,   448,   449,   450,   451,   452,   453,
     454,   455,   458,   458,   466,   466,   476,   477,   480,   480,
     490,   491,   494,   495,   496,   499,   499,   509,   510,   513,
     514,   515,   518,   518,   528,   529,   532,   533,   534,   538,
     543,   548,   548,   555,   556,   557,   558,   561,   561,   569,
     569,   577,   582,   591,   591,   603,   604,   608,   608,   616,
     617,   620,   621,   622,   623,   624,   625,   626,   629,   629,
     637,   642,   642,   650,   650,   660,   661,   664,   664,   672,
     673,   676,   677,   678,   679,   680,   683,   683,   691,   696,
     701,   706,   706
  };

  // Print the state stack on the debug stream.
  void
  NetconfParser::yystack_print_ ()
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
  NetconfParser::yy_reduce_print_ (int yyrule)
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
#endif // NETCONF_DEBUG


#line 14 "netconf_parser.yy"
} } // isc::netconf
#line 2178 "netconf_parser.cc"

#line 714 "netconf_parser.yy"


void
isc::netconf::NetconfParser::error(const location_type& loc,
                               const std::string& what)
{
    ctx.error(loc, what);
}
