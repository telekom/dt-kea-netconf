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
#define yylex   agent_lex



#include "agent_parser.h"


// Unqualified %code blocks.
#line 33 "agent_parser.yy"

#include <agent/parser_context.h>

#line 51 "agent_parser.cc"


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
#if AGENT_DEBUG

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

#else // !AGENT_DEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !AGENT_DEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 14 "agent_parser.yy"
namespace isc { namespace agent {
#line 143 "agent_parser.cc"


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  AgentParser::yytnamerr_ (const char *yystr)
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
  AgentParser::AgentParser (isc::agent::ParserContext& ctx_yyarg)
#if AGENT_DEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      ctx (ctx_yyarg)
  {}

  AgentParser::~AgentParser ()
  {}

  AgentParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  AgentParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  AgentParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  AgentParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  AgentParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  AgentParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  AgentParser::symbol_number_type
  AgentParser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  AgentParser::stack_symbol_type::stack_symbol_type ()
  {}

  AgentParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 51: // value
      case 54: // map_value
      case 107: // socket_type_value
        value.YY_MOVE_OR_COPY< ElementPtr > (YY_MOVE (that.value));
        break;

      case 42: // "boolean"
        value.YY_MOVE_OR_COPY< bool > (YY_MOVE (that.value));
        break;

      case 41: // "floating point"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case 40: // "integer"
        value.YY_MOVE_OR_COPY< int64_t > (YY_MOVE (that.value));
        break;

      case 39: // "constant string"
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

  AgentParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.type_get ())
    {
      case 51: // value
      case 54: // map_value
      case 107: // socket_type_value
        value.move< ElementPtr > (YY_MOVE (that.value));
        break;

      case 42: // "boolean"
        value.move< bool > (YY_MOVE (that.value));
        break;

      case 41: // "floating point"
        value.move< double > (YY_MOVE (that.value));
        break;

      case 40: // "integer"
        value.move< int64_t > (YY_MOVE (that.value));
        break;

      case 39: // "constant string"
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  AgentParser::stack_symbol_type&
  AgentParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 51: // value
      case 54: // map_value
      case 107: // socket_type_value
        value.copy< ElementPtr > (that.value);
        break;

      case 42: // "boolean"
        value.copy< bool > (that.value);
        break;

      case 41: // "floating point"
        value.copy< double > (that.value);
        break;

      case 40: // "integer"
        value.copy< int64_t > (that.value);
        break;

      case 39: // "constant string"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  AgentParser::stack_symbol_type&
  AgentParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 51: // value
      case 54: // map_value
      case 107: // socket_type_value
        value.move< ElementPtr > (that.value);
        break;

      case 42: // "boolean"
        value.move< bool > (that.value);
        break;

      case 41: // "floating point"
        value.move< double > (that.value);
        break;

      case 40: // "integer"
        value.move< int64_t > (that.value);
        break;

      case 39: // "constant string"
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
  AgentParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if AGENT_DEBUG
  template <typename Base>
  void
  AgentParser::yy_print_ (std::ostream& yyo,
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
      case 39: // "constant string"
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < std::string > (); }
#line 427 "agent_parser.cc"
        break;

      case 40: // "integer"
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < int64_t > (); }
#line 433 "agent_parser.cc"
        break;

      case 41: // "floating point"
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < double > (); }
#line 439 "agent_parser.cc"
        break;

      case 42: // "boolean"
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < bool > (); }
#line 445 "agent_parser.cc"
        break;

      case 51: // value
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 451 "agent_parser.cc"
        break;

      case 54: // map_value
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 457 "agent_parser.cc"
        break;

      case 107: // socket_type_value
#line 100 "agent_parser.yy"
                 { yyoutput << yysym.value.template as < ElementPtr > (); }
#line 463 "agent_parser.cc"
        break;

      default:
        break;
    }
    yyo << ')';
  }
#endif

  void
  AgentParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  AgentParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  AgentParser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if AGENT_DEBUG
  std::ostream&
  AgentParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  AgentParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  AgentParser::debug_level_type
  AgentParser::debug_level () const
  {
    return yydebug_;
  }

  void
  AgentParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // AGENT_DEBUG

  AgentParser::state_type
  AgentParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  AgentParser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  AgentParser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  AgentParser::operator() ()
  {
    return parse ();
  }

  int
  AgentParser::parse ()
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
      case 51: // value
      case 54: // map_value
      case 107: // socket_type_value
        yylhs.value.emplace< ElementPtr > ();
        break;

      case 42: // "boolean"
        yylhs.value.emplace< bool > ();
        break;

      case 41: // "floating point"
        yylhs.value.emplace< double > ();
        break;

      case 40: // "integer"
        yylhs.value.emplace< int64_t > ();
        break;

      case 39: // "constant string"
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
#line 111 "agent_parser.yy"
                       { ctx.ctx_ = ctx.NO_KEYWORDS; }
#line 726 "agent_parser.cc"
    break;

  case 4:
#line 112 "agent_parser.yy"
                       { ctx.ctx_ = ctx.CONFIG; }
#line 732 "agent_parser.cc"
    break;

  case 6:
#line 113 "agent_parser.yy"
                       { ctx.ctx_ = ctx.AGENT; }
#line 738 "agent_parser.cc"
    break;

  case 8:
#line 121 "agent_parser.yy"
                          {
    // Parse the Control-agent map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 748 "agent_parser.cc"
    break;

  case 9:
#line 125 "agent_parser.yy"
                               {
    // parsing completed
}
#line 756 "agent_parser.cc"
    break;

  case 10:
#line 132 "agent_parser.yy"
            {
    // Push back the JSON value on the stack
    ctx.stack_.push_back(yystack_[0].value.as < ElementPtr > ());
}
#line 765 "agent_parser.cc"
    break;

  case 11:
#line 138 "agent_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location))); }
#line 771 "agent_parser.cc"
    break;

  case 12:
#line 139 "agent_parser.yy"
             { yylhs.value.as < ElementPtr > () = ElementPtr(new DoubleElement(yystack_[0].value.as < double > (), ctx.loc2pos(yystack_[0].location))); }
#line 777 "agent_parser.cc"
    break;

  case 13:
#line 140 "agent_parser.yy"
               { yylhs.value.as < ElementPtr > () = ElementPtr(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location))); }
#line 783 "agent_parser.cc"
    break;

  case 14:
#line 141 "agent_parser.yy"
              { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location))); }
#line 789 "agent_parser.cc"
    break;

  case 15:
#line 142 "agent_parser.yy"
                 { yylhs.value.as < ElementPtr > () = ElementPtr(new NullElement(ctx.loc2pos(yystack_[0].location))); }
#line 795 "agent_parser.cc"
    break;

  case 16:
#line 143 "agent_parser.yy"
           { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 801 "agent_parser.cc"
    break;

  case 17:
#line 144 "agent_parser.yy"
                    { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 807 "agent_parser.cc"
    break;

  case 18:
#line 148 "agent_parser.yy"
                    {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 818 "agent_parser.cc"
    break;

  case 19:
#line 153 "agent_parser.yy"
                             {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
}
#line 828 "agent_parser.cc"
    break;

  case 20:
#line 159 "agent_parser.yy"
               { yylhs.value.as < ElementPtr > () = ctx.stack_.back(); ctx.stack_.pop_back(); }
#line 834 "agent_parser.cc"
    break;

  case 23:
#line 173 "agent_parser.yy"
                                  {
                  // map containing a single entry
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 843 "agent_parser.cc"
    break;

  case 24:
#line 177 "agent_parser.yy"
                                                      {
                  // map consisting of a shorter map followed by
                  // comma and string:value
                  ctx.stack_.back()->set(yystack_[2].value.as < std::string > (), yystack_[0].value.as < ElementPtr > ());
                  }
#line 853 "agent_parser.cc"
    break;

  case 25:
#line 184 "agent_parser.yy"
                              {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(l);
}
#line 862 "agent_parser.cc"
    break;

  case 26:
#line 187 "agent_parser.yy"
                               {
}
#line 869 "agent_parser.cc"
    break;

  case 29:
#line 194 "agent_parser.yy"
                      {
                  // List consisting of a single element.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 878 "agent_parser.cc"
    break;

  case 30:
#line 198 "agent_parser.yy"
                                           {
                  // List ending with , and a value.
                  ctx.stack_.back()->add(yystack_[0].value.as < ElementPtr > ());
                  }
#line 887 "agent_parser.cc"
    break;

  case 31:
#line 211 "agent_parser.yy"
                                {
    const std::string& where = ctx.contextName();
    const std::string& keyword = yystack_[1].value.as < std::string > ();
    error(yystack_[1].location,
          "got unexpected keyword \"" + keyword + "\" in " + where + " map.");
}
#line 898 "agent_parser.cc"
    break;

  case 32:
#line 219 "agent_parser.yy"
                                 {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.push_back(m);
}
#line 909 "agent_parser.cc"
    break;

  case 33:
#line 224 "agent_parser.yy"
                                {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
}
#line 919 "agent_parser.cc"
    break;

  case 36:
#line 239 "agent_parser.yy"
                            {

    // Let's create a MapElement that will represent it, add it to the
    // top level map (that's already on the stack) and put the new map
    // on the stack as well, so child elements will be able to add
    // themselves to it.
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("Control-agent", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.AGENT);
}
#line 935 "agent_parser.cc"
    break;

  case 37:
#line 249 "agent_parser.yy"
                                                    {
    // Ok, we're done with parsing control-agent. Let's take the map
    // off the stack.
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 946 "agent_parser.cc"
    break;

  case 48:
#line 272 "agent_parser.yy"
                     {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 954 "agent_parser.cc"
    break;

  case 49:
#line 274 "agent_parser.yy"
               {
    ElementPtr host(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("http-host", host);
    ctx.leave();
}
#line 964 "agent_parser.cc"
    break;

  case 50:
#line 280 "agent_parser.yy"
                                   {
    ElementPtr prf(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("http-port", prf);
}
#line 973 "agent_parser.cc"
    break;

  case 51:
#line 285 "agent_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 981 "agent_parser.cc"
    break;

  case 52:
#line 287 "agent_parser.yy"
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
#line 1008 "agent_parser.cc"
    break;

  case 53:
#line 310 "agent_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1016 "agent_parser.cc"
    break;

  case 54:
#line 312 "agent_parser.yy"
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
#line 1045 "agent_parser.cc"
    break;

  case 55:
#line 338 "agent_parser.yy"
                                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("hooks-libraries", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOOKS_LIBRARIES);
}
#line 1056 "agent_parser.cc"
    break;

  case 56:
#line 343 "agent_parser.yy"
                                                             {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1065 "agent_parser.cc"
    break;

  case 61:
#line 356 "agent_parser.yy"
                              {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1075 "agent_parser.cc"
    break;

  case 62:
#line 360 "agent_parser.yy"
                              {
    ctx.stack_.pop_back();
}
#line 1083 "agent_parser.cc"
    break;

  case 69:
#line 374 "agent_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1091 "agent_parser.cc"
    break;

  case 70:
#line 376 "agent_parser.yy"
               {
    ElementPtr lib(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("library", lib);
    ctx.leave();
}
#line 1101 "agent_parser.cc"
    break;

  case 71:
#line 382 "agent_parser.yy"
                           {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1109 "agent_parser.cc"
    break;

  case 72:
#line 384 "agent_parser.yy"
               {
    ElementPtr lib(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("library-path", lib);
    ctx.leave();
}
#line 1119 "agent_parser.cc"
    break;

  case 73:
#line 390 "agent_parser.yy"
                       {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1127 "agent_parser.cc"
    break;

  case 74:
#line 392 "agent_parser.yy"
                  {
    ctx.stack_.back()->set("parameters", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1136 "agent_parser.cc"
    break;

  case 75:
#line 400 "agent_parser.yy"
                                                      {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[2].location)));
    ctx.stack_.back()->set("control-sockets", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKETS);
}
#line 1147 "agent_parser.cc"
    break;

  case 76:
#line 405 "agent_parser.yy"
                                        {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1156 "agent_parser.cc"
    break;

  case 83:
#line 426 "agent_parser.yy"
                                  {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp4", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.SERVER);
}
#line 1167 "agent_parser.cc"
    break;

  case 84:
#line 431 "agent_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1176 "agent_parser.cc"
    break;

  case 85:
#line 437 "agent_parser.yy"
                                  {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("dhcp6", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.SERVER);
}
#line 1187 "agent_parser.cc"
    break;

  case 86:
#line 442 "agent_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1196 "agent_parser.cc"
    break;

  case 87:
#line 448 "agent_parser.yy"
                            {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("d2", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.SERVER);
}
#line 1207 "agent_parser.cc"
    break;

  case 88:
#line 453 "agent_parser.yy"
                                                            {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1216 "agent_parser.cc"
    break;

  case 96:
#line 472 "agent_parser.yy"
                         {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1224 "agent_parser.cc"
    break;

  case 97:
#line 474 "agent_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("socket-name", name);
    ctx.leave();
}
#line 1234 "agent_parser.cc"
    break;

  case 98:
#line 481 "agent_parser.yy"
                         {
    ctx.enter(ctx.SOCKET_TYPE);
}
#line 1242 "agent_parser.cc"
    break;

  case 99:
#line 483 "agent_parser.yy"
                          {
    ctx.stack_.back()->set("socket-type", yystack_[0].value.as < ElementPtr > ());
    ctx.leave();
}
#line 1251 "agent_parser.cc"
    break;

  case 100:
#line 489 "agent_parser.yy"
                         { yylhs.value.as < ElementPtr > () = ElementPtr(new StringElement("unix", ctx.loc2pos(yystack_[0].location))); }
#line 1257 "agent_parser.cc"
    break;

  case 101:
#line 496 "agent_parser.yy"
                 {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("loggers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.LOGGERS);
}
#line 1268 "agent_parser.cc"
    break;

  case 102:
#line 501 "agent_parser.yy"
                                                         {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1277 "agent_parser.cc"
    break;

  case 105:
#line 513 "agent_parser.yy"
                             {
    ElementPtr l(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(l);
    ctx.stack_.push_back(l);
}
#line 1287 "agent_parser.cc"
    break;

  case 106:
#line 517 "agent_parser.yy"
                               {
    ctx.stack_.pop_back();
}
#line 1295 "agent_parser.cc"
    break;

  case 116:
#line 534 "agent_parser.yy"
           {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1303 "agent_parser.cc"
    break;

  case 117:
#line 536 "agent_parser.yy"
               {
    ElementPtr name(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("name", name);
    ctx.leave();
}
#line 1313 "agent_parser.cc"
    break;

  case 118:
#line 542 "agent_parser.yy"
                                     {
    ElementPtr dl(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("debuglevel", dl);
}
#line 1322 "agent_parser.cc"
    break;

  case 119:
#line 547 "agent_parser.yy"
                   {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1330 "agent_parser.cc"
    break;

  case 120:
#line 549 "agent_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("severity", sev);
    ctx.leave();
}
#line 1340 "agent_parser.cc"
    break;

  case 121:
#line 555 "agent_parser.yy"
                                    {
    ElementPtr l(new ListElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output_options", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OUTPUT_OPTIONS);
}
#line 1351 "agent_parser.cc"
    break;

  case 122:
#line 560 "agent_parser.yy"
                                                                    {
    ctx.stack_.pop_back();
    ctx.leave();
}
#line 1360 "agent_parser.cc"
    break;

  case 125:
#line 569 "agent_parser.yy"
                             {
    ElementPtr m(new MapElement(ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
}
#line 1370 "agent_parser.cc"
    break;

  case 126:
#line 573 "agent_parser.yy"
                                    {
    ctx.stack_.pop_back();
}
#line 1378 "agent_parser.cc"
    break;

  case 134:
#line 588 "agent_parser.yy"
               {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1386 "agent_parser.cc"
    break;

  case 135:
#line 590 "agent_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("output", sev);
    ctx.leave();
}
#line 1396 "agent_parser.cc"
    break;

  case 136:
#line 596 "agent_parser.yy"
                           {
    ElementPtr flush(new BoolElement(yystack_[0].value.as < bool > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("flush", flush);
}
#line 1405 "agent_parser.cc"
    break;

  case 137:
#line 601 "agent_parser.yy"
                               {
    ElementPtr maxsize(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxsize", maxsize);
}
#line 1414 "agent_parser.cc"
    break;

  case 138:
#line 606 "agent_parser.yy"
                             {
    ElementPtr maxver(new IntElement(yystack_[0].value.as < int64_t > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("maxver", maxver);
}
#line 1423 "agent_parser.cc"
    break;

  case 139:
#line 611 "agent_parser.yy"
                 {
    ctx.enter(ctx.NO_KEYWORDS);
}
#line 1431 "agent_parser.cc"
    break;

  case 140:
#line 613 "agent_parser.yy"
               {
    ElementPtr sev(new StringElement(yystack_[0].value.as < std::string > (), ctx.loc2pos(yystack_[0].location)));
    ctx.stack_.back()->set("pattern", sev);
    ctx.leave();
}
#line 1441 "agent_parser.cc"
    break;


#line 1445 "agent_parser.cc"

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
  AgentParser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  AgentParser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
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


  const short AgentParser::yypact_ninf_ = -133;

  const signed char AgentParser::yytable_ninf_ = -1;

  const short
  AgentParser::yypact_[] =
  {
      -5,  -133,  -133,  -133,     6,     0,     1,    48,  -133,  -133,
    -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,
    -133,  -133,  -133,  -133,     0,    10,    46,     4,  -133,    82,
      89,    93,    90,    96,  -133,    92,  -133,  -133,  -133,    97,
    -133,  -133,    98,  -133,  -133,   100,  -133,    26,  -133,  -133,
    -133,  -133,  -133,  -133,  -133,  -133,  -133,     0,     0,  -133,
      66,   102,  -133,   103,    68,   105,   106,   104,   108,   109,
    -133,     4,  -133,  -133,  -133,   110,   111,    80,  -133,   113,
      83,  -133,   112,   116,  -133,     0,     4,  -133,  -133,  -133,
    -133,    -4,   117,   118,  -133,    54,  -133,  -133,  -133,  -133,
      55,  -133,  -133,  -133,  -133,  -133,   120,   124,  -133,  -133,
      17,  -133,  -133,   119,   125,   126,    -4,  -133,    27,  -133,
     117,    -3,   118,  -133,   121,   127,   128,  -133,  -133,  -133,
    -133,  -133,    64,  -133,  -133,  -133,  -133,  -133,  -133,  -133,
     129,  -133,  -133,  -133,  -133,    75,  -133,  -133,  -133,  -133,
    -133,  -133,    34,    34,    34,   132,   136,   137,    45,  -133,
     138,   139,    91,   140,    -3,  -133,  -133,  -133,  -133,  -133,
    -133,    76,  -133,  -133,  -133,    77,    78,   107,   114,   113,
    -133,   115,   142,  -133,   122,  -133,   141,   144,    34,  -133,
    -133,  -133,  -133,  -133,  -133,  -133,   143,  -133,   123,    94,
    -133,  -133,    87,  -133,  -133,  -133,  -133,    42,   143,  -133,
    -133,   145,   147,   148,  -133,    79,  -133,  -133,  -133,  -133,
    -133,  -133,  -133,   151,    74,   130,   131,   152,    42,  -133,
     133,  -133,  -133,  -133,   134,  -133,  -133,  -133
  };

  const unsigned char
  AgentParser::yydefact_[] =
  {
       0,     2,     4,     6,     0,     0,     0,     0,     1,    25,
      18,    15,    14,    11,    12,    13,     3,    10,    16,    17,
      32,     5,     8,     7,    27,    21,     0,     0,    29,     0,
      28,     0,     0,    22,    36,     0,    34,    35,    48,     0,
      51,    53,     0,    55,   101,     0,    47,     0,    38,    40,
      41,    45,    46,    43,    42,    44,    26,     0,     0,    19,
       0,     0,    33,     0,     0,     0,     0,     0,     0,     0,
      31,     0,     9,    30,    23,     0,     0,     0,    50,     0,
       0,    75,     0,     0,    39,     0,     0,    49,    20,    52,
      54,     0,    57,     0,    24,     0,    83,    85,    87,    82,
       0,    77,    79,    80,    81,    61,     0,    58,    59,   105,
       0,   103,    37,     0,     0,     0,     0,    76,     0,    56,
       0,     0,     0,   102,     0,     0,     0,    78,    69,    71,
      73,    65,     0,    63,    66,    67,    68,    60,   116,   121,
       0,   119,   115,   113,   114,     0,   107,   109,   111,   112,
     110,   104,     0,     0,     0,     0,     0,     0,     0,    62,
       0,     0,     0,     0,     0,   106,    96,    98,    95,    93,
      94,     0,    89,    91,    92,     0,     0,     0,     0,     0,
      64,     0,     0,   118,     0,   108,     0,     0,     0,    84,
      86,    88,    70,    72,    74,   117,     0,   120,     0,     0,
      90,   125,     0,   123,    97,   100,    99,     0,     0,   122,
     134,     0,     0,     0,   139,     0,   127,   129,   130,   131,
     132,   133,   124,     0,     0,     0,     0,     0,     0,   126,
       0,   136,   137,   138,     0,   128,   135,   140
  };

  const short
  AgentParser::yypgoto_[] =
  {
    -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,   -20,   -76,
    -133,   -47,  -133,  -133,  -133,  -133,  -133,  -133,   -27,  -133,
    -133,  -133,  -133,  -133,  -133,    71,    88,  -133,  -133,  -133,
     -26,  -133,   -25,  -133,  -133,  -133,  -133,  -133,    38,  -133,
    -133,     2,  -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,
    -133,    47,  -133,  -133,  -133,  -133,  -133,  -133,  -132,   -24,
    -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,    43,  -133,
    -133,     3,  -133,  -133,  -133,  -133,  -133,  -133,  -133,  -133,
     -42,  -133,  -133,   -60,  -133,  -133,  -133,  -133,  -133,  -133,
    -133
  };

  const short
  AgentParser::yydefgoto_[] =
  {
      -1,     4,     5,     6,     7,    23,    27,    16,    17,    18,
      25,    89,    32,    33,    19,    24,    29,    30,   168,    21,
      26,    35,    36,    37,    61,    47,    48,    49,    63,    50,
     169,    65,   170,    66,    53,    68,   106,   107,   108,   118,
     132,   133,   134,   155,   135,   156,   136,   157,    54,    91,
     100,   101,   102,   113,   103,   114,   104,   115,   171,   172,
     173,   186,   174,   187,   206,    55,    69,   110,   111,   121,
     145,   146,   147,   160,   148,   149,   163,   150,   161,   202,
     203,   207,   215,   216,   217,   223,   218,   219,   220,   221,
     227
  };

  const unsigned char
  AgentParser::yytable_[] =
  {
      46,    51,    52,    88,    28,     9,     8,    10,    20,    11,
      40,    41,    96,    97,    98,    38,    39,    40,    41,    42,
     122,   175,   176,   123,   138,   139,    43,   140,   141,    71,
      44,     1,     2,     3,    72,    45,    45,    73,    74,    12,
      13,    14,    15,    45,    46,    51,    52,    40,    41,    31,
     128,   129,   130,   166,   167,    22,    34,    71,   116,    46,
      51,    52,   112,   117,    99,    94,    45,   158,   128,   129,
     130,   210,   159,    45,   211,   212,   213,   214,   164,   188,
     188,   188,   228,   165,   189,   190,   191,   229,    56,    99,
     208,   131,    57,   209,   142,   143,   144,    58,    59,    60,
      62,    64,    67,    88,    70,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    85,   205,   231,    92,    86,    87,
      10,    93,    90,   124,   105,   109,   119,   120,   152,   125,
     126,   183,   194,   162,   153,   154,   177,   142,   143,   144,
     178,   179,   181,   182,   184,   198,   192,   196,   199,   224,
     201,   225,   226,   193,   195,   230,   234,    95,   137,    84,
     180,   197,   204,   127,   200,   151,   222,   185,   235,     0,
     232,   233,   236,   237
  };

  const short
  AgentParser::yycheck_[] =
  {
      27,    27,    27,    79,    24,     5,     0,     7,     7,     9,
      13,    14,    16,    17,    18,    11,    12,    13,    14,    15,
       3,   153,   154,     6,    27,    28,    22,    30,    31,     3,
      26,    36,    37,    38,     8,    39,    39,    57,    58,    39,
      40,    41,    42,    39,    71,    71,    71,    13,    14,    39,
      23,    24,    25,    19,    20,     7,    10,     3,     3,    86,
      86,    86,     8,     8,    91,    85,    39,     3,    23,    24,
      25,    29,     8,    39,    32,    33,    34,    35,     3,     3,
       3,     3,     3,     8,     8,     8,     8,     8,     6,   116,
       3,   118,     3,     6,   121,   121,   121,     4,     8,     3,
       8,     4,     4,   179,     4,    39,     4,     4,    40,     4,
       4,     7,     4,     4,     4,    21,    42,     5,     7,    39,
       7,     5,    39,     4,     7,     7,     6,     3,     7,     4,
       4,    40,   179,     4,     7,     7,     4,   164,   164,   164,
       4,     4,     4,     4,     4,     4,    39,     5,     4,     4,
       7,     4,     4,    39,    39,     4,     4,    86,   120,    71,
     158,    39,    39,   116,   188,   122,   208,   164,   228,    -1,
      40,    40,    39,    39
  };

  const unsigned char
  AgentParser::yystos_[] =
  {
       0,    36,    37,    38,    44,    45,    46,    47,     0,     5,
       7,     9,    39,    40,    41,    42,    50,    51,    52,    57,
       7,    62,     7,    48,    58,    53,    63,    49,    51,    59,
      60,    39,    55,    56,    10,    64,    65,    66,    11,    12,
      13,    14,    15,    22,    26,    39,    61,    68,    69,    70,
      72,    73,    75,    77,    91,   108,     6,     3,     4,     8,
       3,    67,     8,    71,     4,    74,    76,     4,    78,   109,
       4,     3,     8,    51,    51,    39,     4,     4,    40,     4,
       4,     7,     4,     4,    69,     4,     7,    39,    52,    54,
      39,    92,     5,     5,    51,    68,    16,    17,    18,    61,
      93,    94,    95,    97,    99,     7,    79,    80,    81,     7,
     110,   111,     8,    96,    98,   100,     3,     8,    82,     6,
       3,   112,     3,     6,     4,     4,     4,    94,    23,    24,
      25,    61,    83,    84,    85,    87,    89,    81,    27,    28,
      30,    31,    61,    73,    75,   113,   114,   115,   117,   118,
     120,   111,     7,     7,     7,    86,    88,    90,     3,     8,
     116,   121,     4,   119,     3,     8,    19,    20,    61,    73,
      75,   101,   102,   103,   105,   101,   101,     4,     4,     4,
      84,     4,     4,    40,     4,   114,   104,   106,     3,     8,
       8,     8,    39,    39,    54,    39,     5,    39,     4,     4,
     102,     7,   122,   123,    39,    21,   107,   124,     3,     6,
      29,    32,    33,    34,    35,   125,   126,   127,   129,   130,
     131,   132,   123,   128,     4,     4,     4,   133,     3,     8,
       4,    42,    40,    40,     4,   126,    39,    39
  };

  const unsigned char
  AgentParser::yyr1_[] =
  {
       0,    43,    45,    44,    46,    44,    47,    44,    49,    48,
      50,    51,    51,    51,    51,    51,    51,    51,    53,    52,
      54,    55,    55,    56,    56,    58,    57,    59,    59,    60,
      60,    61,    63,    62,    64,    65,    67,    66,    68,    68,
      69,    69,    69,    69,    69,    69,    69,    69,    71,    70,
      72,    74,    73,    76,    75,    78,    77,    79,    79,    80,
      80,    82,    81,    83,    83,    83,    84,    84,    84,    86,
      85,    88,    87,    90,    89,    92,    91,    93,    93,    94,
      94,    94,    94,    96,    95,    98,    97,   100,    99,   101,
     101,   102,   102,   102,   102,   102,   104,   103,   106,   105,
     107,   109,   108,   110,   110,   112,   111,   113,   113,   114,
     114,   114,   114,   114,   114,   114,   116,   115,   117,   119,
     118,   121,   120,   122,   122,   124,   123,   125,   125,   126,
     126,   126,   126,   126,   128,   127,   129,   130,   131,   133,
     132
  };

  const signed char
  AgentParser::yyr2_[] =
  {
       0,     2,     0,     3,     0,     3,     0,     3,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     4,
       1,     0,     1,     3,     5,     0,     4,     0,     1,     1,
       3,     2,     0,     4,     1,     1,     0,     6,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     4,
       3,     0,     4,     0,     4,     0,     6,     0,     1,     1,
       3,     0,     4,     1,     3,     1,     1,     1,     1,     0,
       4,     0,     4,     0,     4,     0,     6,     1,     3,     1,
       1,     1,     1,     0,     6,     0,     6,     0,     6,     1,
       3,     1,     1,     1,     1,     1,     0,     4,     0,     4,
       1,     0,     6,     1,     3,     0,     4,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     4,     3,     0,
       4,     0,     6,     1,     3,     0,     4,     1,     3,     1,
       1,     1,     1,     1,     0,     4,     3,     3,     3,     0,
       4
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const AgentParser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\",\"", "\":\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"null\"", "\"Control-agent\"",
  "\"http-host\"", "\"http-port\"", "\"user-context\"", "\"comment\"",
  "\"control-sockets\"", "\"dhcp4\"", "\"dhcp6\"", "\"d2\"",
  "\"socket-name\"", "\"socket-type\"", "\"unix\"", "\"hooks-libraries\"",
  "\"library\"", "\"library-path\"", "\"parameters\"", "\"loggers\"",
  "\"name\"", "\"output_options\"", "\"output\"", "\"debuglevel\"",
  "\"severity\"", "\"flush\"", "\"maxsize\"", "\"maxver\"", "\"pattern\"",
  "START_JSON", "START_AGENT", "START_SUB_AGENT", "\"constant string\"",
  "\"integer\"", "\"floating point\"", "\"boolean\"", "$accept", "start",
  "$@1", "$@2", "$@3", "sub_agent", "$@4", "json", "value", "map", "$@5",
  "map_value", "map_content", "not_empty_map", "list_generic", "$@6",
  "list_content", "not_empty_list", "unknown_map_entry",
  "agent_syntax_map", "$@7", "global_objects", "global_object",
  "agent_object", "$@8", "global_params", "global_param", "http_host",
  "$@9", "http_port", "user_context", "$@10", "comment", "$@11",
  "hooks_libraries", "$@12", "hooks_libraries_list",
  "not_empty_hooks_libraries_list", "hooks_library", "$@13",
  "hooks_params", "hooks_param", "library", "$@14", "library-path", "$@15",
  "parameters", "$@16", "control_sockets", "$@17",
  "control_sockets_params", "control_socket", "dhcp4_server_socket",
  "$@18", "dhcp6_server_socket", "$@19", "d2_server_socket", "$@20",
  "control_socket_params", "control_socket_param", "socket_name", "$@21",
  "socket_type", "$@22", "socket_type_value", "loggers", "$@23",
  "loggers_entries", "logger_entry", "$@24", "logger_params",
  "logger_param", "name", "$@25", "debuglevel", "severity", "$@26",
  "output_options_list", "$@27", "output_options_list_content",
  "output_entry", "$@28", "output_params_list", "output_params", "output",
  "$@29", "flush", "maxsize", "maxver", "pattern", "$@30", YY_NULLPTR
  };

#if AGENT_DEBUG
  const short
  AgentParser::yyrline_[] =
  {
       0,   111,   111,   111,   112,   112,   113,   113,   121,   121,
     132,   138,   139,   140,   141,   142,   143,   144,   148,   148,
     159,   164,   165,   173,   177,   184,   184,   190,   191,   194,
     198,   211,   219,   219,   231,   235,   239,   239,   256,   257,
     262,   263,   264,   265,   266,   267,   268,   269,   272,   272,
     280,   285,   285,   310,   310,   338,   338,   348,   349,   352,
     353,   356,   356,   364,   365,   366,   369,   370,   371,   374,
     374,   382,   382,   390,   390,   400,   400,   413,   414,   419,
     420,   421,   422,   426,   426,   437,   437,   448,   448,   459,
     460,   464,   465,   466,   467,   468,   472,   472,   481,   481,
     489,   496,   496,   508,   509,   513,   513,   521,   522,   525,
     526,   527,   528,   529,   530,   531,   534,   534,   542,   547,
     547,   555,   555,   565,   566,   569,   569,   577,   578,   581,
     582,   583,   584,   585,   588,   588,   596,   601,   606,   611,
     611
  };

  // Print the state stack on the debug stream.
  void
  AgentParser::yystack_print_ ()
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
  AgentParser::yy_reduce_print_ (int yyrule)
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
#endif // AGENT_DEBUG


#line 14 "agent_parser.yy"
} } // isc::agent
#line 2014 "agent_parser.cc"

#line 619 "agent_parser.yy"


void
isc::agent::AgentParser::error(const location_type& loc,
                               const std::string& what)
{
    ctx.error(loc, what);
}
