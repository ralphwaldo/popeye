/********************* MODIFICATIONS to py1.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/01 SE   New Koeko conditions: GI-Koeko, AN-Koeko
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New condition: Protean Chess
 **
 ** 2008/01/11 SE   New variant: Special Grids
 **
 ** 2008/01/24 SE   New variant: Gridlines
 **
 ** 2008/02/24 SE   Bugfix: Koeko + Parrain
 **
 ** 2008/02/19 SE   New condition: AntiKoeko
 **
 ** 2008/02/25 SE   New piece type: Magic
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 ** 2009/04/25 SE   New condition: Provacateurs
 **                 New piece type: Patrol pieces
 **
 **************************** End of List ******************************/

#if defined(macintosh)          /* is always defined on macintosh's  SB */
#    define SEGM1
#    include "platform/unix/mac.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "py.h"
#include "pymsg.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/killer_move/killer_move.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/movenumbers.h"
#include "options/maxflightsquares.h"
#include "stipulation/stipulation.h"
#include "pieces/hunters.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/attributes/magic.h"
#include "position/pieceid.h"
#include "platform/maxtime.h"
#include "solving/move_effect_journal.h"
#include "solving/battle_play/try.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/king_capture_avoider.h"
#include "conditions/bgl.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/duellists.h"
#include "conditions/kobul.h"
#include "conditions/anticirce/super.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/april.h"
#include "conditions/circe/takemake.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/imitator.h"
#include "conditions/sentinelles.h"
#include "conditions/mummer.h"
#include "conditions/singlebox/type3.h"
#include "conditions/immune.h"
#include "conditions/geneva.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/phantom.h"
#include "conditions/annan.h"
#include "conditions/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "utilities/table.h"
#include "debugging/trace.h"

static numvec ortho_opt[4][2*(square_h8-square_a1)+1];

static numvec const * const check_dir_impl[4] = {
    ortho_opt[Queen-Queen]+(square_h8-square_a1),
    ortho_opt[Knight-Queen]+(square_h8-square_a1),
    ortho_opt[Rook-Queen]+(square_h8-square_a1),
    ortho_opt[Bishop-Queen]+(square_h8-square_a1)
};

numvec const * const * const CheckDir = check_dir_impl-Queen;

void InitCheckDir(void)
{
  vec_index_type i;
  unsigned int j;

  assert(Queen<Rook);
  assert(Rook-Queen<4);
  assert(Queen<Bishop);
  assert(Bishop-Queen<4);
  assert(Queen<Knight);
  assert(Knight-Queen<4);

  for (i = -(square_h8-square_a1); i<=square_h8-square_a1; i++)
  {
    ortho_opt[Queen-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Rook-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Bishop-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Knight-Queen][(square_h8-square_a1)+i] = 0;
  }

  for (i = vec_knight_start; i <= vec_knight_end; i++)
    ortho_opt[Knight-Queen][(square_h8-square_a1)+vec[i]] = vec[i];

  for (i = vec_rook_start; i<=vec_rook_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Rook-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }

  for (i = vec_bishop_start; i<=vec_bishop_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Bishop-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }
}

static void initply(ply parent, ply child)
{
  parent_ply[child] = parent;

  /* child -1 is correct and parent would be wrong! */
  move_effect_journal_top[child] = move_effect_journal_top[child-1];

  prev_king_square[White][nbply] = king_square[White];
  prev_king_square[Black][nbply] = king_square[Black];

  /*
    start with the castling rights of the parent level
  */
  castling_flag[child] = castling_flag[parent];

  /*
    start with the SAT state of the parent level
  */
  StrictSAT[Black][child] = StrictSAT[Black][parent];
  StrictSAT[White][child] = StrictSAT[White][parent];
  BGL_values[White][child] = BGL_values[White][parent];
  BGL_values[Black][child] = BGL_values[Black][parent];

  magic_views_top[child] = magic_views_top[child-1];

  platzwechsel_rochade_allowed[White][child] = platzwechsel_rochade_allowed[White][parent];
  platzwechsel_rochade_allowed[Black][child] = platzwechsel_rochade_allowed[Black][parent];

  take_make_circe_current_rebirth_square_index[child] = take_make_circe_current_rebirth_square_index[parent];

  ++post_move_iteration_id[child];

  TraceValue("%u",child);
  TraceValue("%u",move_effect_journal_top[child]);
  TraceValue("%u\n",post_move_iteration_id[child]);
}

static void do_copyply(ply original, ply copy)
{
  parent_ply[copy] = parent_ply[original];

  trait[copy] = trait[original];

  move_effect_journal_top[copy] = move_effect_journal_top[copy-1];

  prev_king_square[White][nbply] = prev_king_square[White][parent_ply[original]];
  prev_king_square[Black][nbply] = prev_king_square[Black][parent_ply[original]];

  /*
    start with the castling rights of the parent level
  */
  castling_flag[copy] = castling_flag[parent_ply[original]];

  /*
    start with the SAT state of the original level
  */
  StrictSAT[Black][copy] = StrictSAT[Black][parent_ply[original]];
  StrictSAT[White][copy] = StrictSAT[White][parent_ply[original]];
  BGL_values[White][copy] = BGL_values[White][parent_ply[original]];
  BGL_values[Black][copy] = BGL_values[Black][parent_ply[original]];

  magic_views_top[copy] = magic_views_top[copy-1];

  platzwechsel_rochade_allowed[White][copy] = platzwechsel_rochade_allowed[White][parent_ply[original]];
  platzwechsel_rochade_allowed[Black][copy] = platzwechsel_rochade_allowed[Black][parent_ply[original]];

  {
    unsigned int const nr_moves = current_move[original]-current_move[original-1];
    memcpy(&move_generation_stack[current_move[copy]+1],
           &move_generation_stack[current_move[original-1]+1],
           nr_moves*sizeof move_generation_stack[0]);
    current_move[copy] += nr_moves;
  }

  ++post_move_iteration_id[copy];
  TraceValue("%u",nbply);TraceValue("%u\n",post_move_iteration_id[nbply]);
}

static ply ply_watermark;

void nextply(void)
{
  ply const parent = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(ply_watermark<maxply);

  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;
  initply(parent,nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void copyply(void)
{
  ply const original = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;
  do_copyply(original,nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void finply()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nbply==ply_watermark);
  --ply_watermark;
  nbply = parent_ply[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void InitCond(void)
{
  square const *bnp;
  square i, j;

  mummer_strictness[White] = mummer_strictness_none;
  mummer_strictness[Black] = mummer_strictness_none;

  anyclone = false;
  anycirprom = false;
  anycirce = false;
  anyimmun = false;
  anyanticirce = false;
  anyanticirprom = false;
  anymars = false;
  anyantimars = false;
  anygeneva = false;
  anyparrain= false;

  antirenai = rennormal;
  circerenai = rennormal;
  immunrenai = rennormal;
  marsrenai = rennormal;

  royal_square[White] = initsquare;
  royal_square[Black] = initsquare;

  CondFlag[circeassassin]= false;
  flagparasent= false;
  rex_mad = false;
  rex_circe = false;
  immune_is_rex_inclusive = false;
  phantom_chess_rex_inclusive = false;
  rex_geneva =false;
  rex_mess_ex = false;
  rex_wooz_ex = false;
  rex_protean_ex = false;
  transmuting_kings_lock_recursion = false;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle[White] = pb;
  sentinelle[Black] = pn;

  gridvar = grid_normal;
  numgridlines = 0;

  {
    PieceIdType id;
    for (id = MinPieceId; id<=MaxPieceId; ++id)
      PiecePositionsInDiagram[id] = initsquare;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    int const file= *bnp%onerow - nr_of_slack_files_left_of_board;
    int const row= *bnp/onerow - nr_of_slack_rows_below_board;

    ClearPieceId(spec[*bnp]);
    CLEARFL(sq_spec[*bnp]);
    sq_num[*bnp]= (int)(bnp-boardnum);

    /* initialise sq_spec and set grid number */
    sq_spec[*bnp] += ((file/2)+4*(row/2)) << Grid;
    if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec[*bnp], NoEdgeSq);
  }

  for (i= square_a1; i < square_h8; i+= onerow)
  {
    if (i > square_a1)
      if (!TSTFLAG(sq_spec[i+dir_down], SqColor))
        SETFLAG(sq_spec[i], SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec[j+dir_left], SqColor))
        SETFLAG(sq_spec[j], SqColor);
  }

  for (i= 0; i < CondCount; ++i)
    CondFlag[i] = false;

  for (i= 0; i < ExtraCondCount; ++i)
    ExtraCondFlag[i] = false;

  number_of_imitators = 0;

  memset((char *) promonly, 0, sizeof(promonly));
  memset((char *) is_football_substitute, 0, sizeof(promonly));
  memset((char *) is_april_kind,0,sizeof(is_april_kind));
  checkhopim = false;
  koeko_nocontact= nokingcontact;
  antikoeko_nocontact= nokingcontact;
  OscillatingKingsTypeB[White]= false;
  OscillatingKingsTypeB[Black]= false;
  OscillatingKingsTypeC[White]= false;
  OscillatingKingsTypeC[Black]= false;

  BGL_values[White][1] = BGL_infinity;
  BGL_values[Black][1] = BGL_infinity;
  BGL_global= false;

  calc_reflective_king[White] = false;
  calc_reflective_king[Black] = false;

  reset_king_vaulters();

  kobulking[White] = false;
  kobulking[Black] = false;
} /* InitCond */

void InitOpt(void)
{
  {
    Side side;
    square castling;
    for (side = White; side<=Black; ++side)
      for (castling = min_castling; castling<=max_castling; ++castling)
        castling_mutual_exclusive[side][castling-min_castling] = 0;
  }

  castling_flag[castlings_flags_no_castling] = bl_castlings|wh_castlings;

  en_passant_forget_multistep();

  resetOptionMaxtime();

  reset_max_flights();
  set_max_nr_refutations(0);
  reset_restart_number();
  reset_max_threat_length();
  reset_nontrivial_settings();

  {
    unsigned int i;
    for (i = 0; i<OptCount; i++)
      OptFlag[i] = false;
  }

  move_effect_journal_reset_retro_capture();
}

void InitBoard(void)
{
  square i;
  square const *bnp;

  ActTitle[0] = '\0';
  ActAuthor[0] = '\0';
  ActOrigin[0] = '\0';
  ActTwinning[0] = '\0';
  ActAward[0] = '\0';
  ActStip[0] = '\0';

  for (i= maxsquare-1; i>=0; i--)
  {
    e[i] = obs;
    e_ubi[i] = obs;
    e_ubi_mad[i] = obs;
    spec[i] = BorderSpec;
  }

  /* dummy squares for Messigny chess and castling -- must be empty */
  e[messigny_exchange] = vide;
  e[kingside_castling] = vide;
  e[queenside_castling] = vide;
  CLEARFL(spec[messigny_exchange]);
  CLEARFL(spec[kingside_castling]);
  CLEARFL(spec[queenside_castling]);

  for (bnp = boardnum; *bnp; bnp++)
    e[*bnp] = vide;

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  CLEARFL(all_pieces_flags);
  CLEARFL(all_royals_flags);

  CLEARFL(some_pieces_flags);
  SETFLAG(some_pieces_flags,White);
  SETFLAG(some_pieces_flags,Black);

  nrhuntertypes = 0;
} /* InitBoard */

void InitAlways(void) {
  square i;

  nbply = nil_ply;
  current_move[nbply] = nil_coup;
  ply_watermark = nil_ply;

  CondFlag[circeassassin] = false;
  flagfee = false;

  for (i= maxply; i > 0; i--)
  {
    duellists[White][i] = initsquare;
    duellists[Black][i] = initsquare;
    killer_moves[i].departure = initsquare;
    killer_moves[i].arrival = initsquare;
    current_circe_rebirth_square[i] = initsquare;
    trait[i] = White;
    current_anticirce_rebirth_square[i] = initsquare;
    pwcprom[i] = false;
  }

  initialise_neutrals(White);
  reset_tables();
  dont_generate_castling = false;

  takemake_takedeparturesquare= initsquare;
  takemake_takecapturesquare= initsquare;

  reset_max_nr_solutions_per_target_position();

  king_capture_avoiders_reset();
}

square coinequis(square i)
{
  return 75 + (onerow*(((i/onerow)+3)/2) + (((i%onerow)+3)/2));
}

boolean leapcheck(square sq_king,
                  vec_index_type kanf, vec_index_type kend,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */
  vec_index_type k;
  for (k= kanf; k<=kend; k++)
  {
    square const sq_departure= sq_king+vec[k];
    if (abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }

  return false;
}

boolean leapleapcheck(square     sq_king,
                      vec_index_type kanf, vec_index_type kend,
                      int hurdletype,
                      boolean leaf,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */
  vec_index_type  k;

  for (k= kanf; k<= kend; k++)
  {
    square const sq_hurdle= sq_king + vec[k];
    if ((hurdletype==0 && abs(e[sq_hurdle])>obs && TSTFLAG(spec[sq_hurdle],advers(trait[nbply])))
        || (hurdletype ==1 && abs(e[sq_hurdle])>obs))
    {
      vec_index_type k1;
      for (k1= kanf; k1<= kend; k1++)
      {
        square const sq_departure = sq_hurdle + vec[k1];
        if (abs(e[sq_departure])==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && sq_departure!=sq_king
            && (*evaluate)(sq_departure,sq_king,sq_king)
            && imcheck(sq_departure,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean riderhoppercheck(square  sq_king,
                         vec_index_type kanf, vec_index_type kend,
                         PieNam p,
                         int     run_up,
                         int     jump,
                         evalfunction_t *evaluate)
{
  /* detect "check" of a generalised rider-hopper p that runs up
     run_up squares and jumps jump squares. 0 indicates an
     potentially infinite run_up or jump.
     examples:  grasshopper:         run_up: 0   jump: 1
     grasshopper2:      run_up: 0    jump: 2
     contragrasshopper: run_up: 1    jump: 0
     lion:           run_up: 0   jump: 0
  ********/

  piece hurdle, hopper;
  square sq_hurdle;
  vec_index_type k;

  square sq_departure;

  for (k= kanf; k <= kend; k++) {
    if (jump) {
      sq_hurdle= sq_king;
      if (jump>1) {
        int jumped= jump;
        while (--jumped) {
          sq_hurdle+= vec[k];
          if (e[sq_hurdle]!=vide)
            break;
        }

        if (jumped)
          continue;
      }
      sq_hurdle+= vec[k];
      hurdle= e[sq_hurdle];
    }
    else
      /* e.g. lion, contragrashopper */
      finligne(sq_king,vec[k],hurdle,sq_hurdle);

    if (abs(hurdle)>=roib) {
      if (run_up) {
        /* contragrashopper */
        sq_departure= sq_hurdle;
        if (run_up>1) {
          int ran_up= run_up;
          while (--ran_up) {
            sq_hurdle+= vec[k];
            if (e[sq_hurdle]!=vide)
              break;
          }
          if (ran_up)
            continue;
        }
        sq_departure+= vec[k];
        hopper= e[sq_departure];
      }
      else
        /* grashopper, lion */
        finligne(sq_hurdle,vec[k],hopper,sq_departure);

      if (abs(hopper)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-vec[k],-vec[k])))
        return true;
    }
  }
  return false;
} /* end of riderhoppercheck */

boolean ridcheck(square sq_king,
                 vec_index_type kanf, vec_index_type kend,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  /* detect "check" of rider p */
  boolean result = false;
  piece rider;
  vec_index_type k;
  square sq_departure;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TracePiece(p);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,trait[nbply],"\n");
  for (k= kanf; k<= kend; k++)
  {
    finligne(sq_king,vec[k],rider,sq_departure);
    TraceSquare(sq_departure);
    TracePiece(rider);
    TraceValue("%u\n",TSTFLAG(spec[sq_departure],trait[nbply]));
    if (abs(rider)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_king,sq_king)
        && ridimcheck(sq_departure,sq_king,vec[k]))
    {
      result = true;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean marine_rider_check(square   sq_king,
                           vec_index_type kanf, vec_index_type kend,
                           PieNam p,
                           evalfunction_t *evaluate)
{
  /* detect "check" of marin piece p or a locust */
  vec_index_type k;

  for (k= kanf; k<= kend; k++)
  {
    square const sq_arrival= sq_king-vec[k];
    if (e[sq_arrival]==vide)
    {
      piece marine;
      square sq_departure;
      finligne(sq_king,vec[k],marine,sq_departure);
      if (abs(marine)==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && evaluate(sq_departure,sq_arrival,sq_king))
        return true;
    }
  }

  return false;
}

boolean marine_leaper_check(square sq_king,
                            vec_index_type kanf, vec_index_type kend,
                            PieNam p,
                            evalfunction_t *evaluate)
{
  vec_index_type k;
  for (k = kanf; k<=kend; ++k)
  {
    square const sq_arrival = sq_king-vec[k];
    square const sq_departure = sq_king+vec[k];
    if (e[sq_arrival]==vide
        && abs(e[sq_departure])==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && evaluate(sq_departure,sq_arrival,sq_king))
      return true;
  }

  return false;
}

static boolean marine_pawn_test_check(square sq_departure,
                                      square sq_hurdle,
                                      square sq_capture,
                                      PieNam p,
                                      evalfunction_t *evaluate)
{
  boolean result;
  numvec const dir_check = sq_hurdle-sq_departure;
  square const sq_arrival = sq_hurdle+dir_check;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_hurdle);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = (abs(e[sq_departure])==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && e[sq_arrival]==vide
            && evaluate(sq_departure,sq_arrival,sq_capture));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean marine_pawn_check(square sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  numvec const dir_forward_right = dir_forward+dir_right;
  numvec const dir_forward_left = dir_forward+dir_left;

  if (marine_pawn_test_check(sq_king-dir_forward_right,sq_king,sq_king,p,evaluate))
    return true;
  else if (marine_pawn_test_check(sq_king-dir_forward_left,sq_king,sq_king,p,evaluate))
    return true;
  else if (en_passant_test_check(sq_king,dir_forward_right,&marine_pawn_test_check,p,evaluate))
    return true;
  else if (en_passant_test_check(sq_king,dir_forward_left,&marine_pawn_test_check,p,evaluate))
    return true;

  return false;
}

boolean marine_ship_check(square sq_king,
                          PieNam p,
                          evalfunction_t *evaluate)
{
  return marine_pawn_check(sq_king,p,evaluate) || tritoncheck(sq_king,p,evaluate);
}

boolean nogridcontact(square j)
{
  square  j1;
  numvec  k;
  piece   p;

  for (k= 8; k > 0; k--) {
    p= e[j1= j + vec[k]];
    if (p != vide && p != obs && GridLegal(j1, j)) {
      return false;
    }
  }
  return true;
}

static boolean noleapcontact(square sq_arrival, vec_index_type kanf, vec_index_type kend)
{
  boolean result = true;

  vec_index_type k;
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();
  for (k= kanf; k <= kend; k++)
  {
    piece const p = e[sq_arrival+vec[k]];
    /* this is faster than a call to abs() */
    if (p!=obs && p!=vide)
    {
      TraceSquare(sq_arrival+vec[k]);
      TracePiece(e[sq_arrival+vec[k]]);
      TraceText("\n");
      result = false;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean nokingcontact(square ia)
{
  return noleapcontact(ia, vec_queen_start, vec_queen_end);
}

boolean nowazircontact(square ia)
{
  return noleapcontact(ia, vec_rook_start, vec_rook_end);
}

boolean noferscontact(square ia)
{
  return noleapcontact(ia, vec_bishop_start, vec_bishop_end);
}

boolean noknightcontact(square ia)
{
  return noleapcontact(ia, vec_knight_start, vec_knight_end);
}

boolean nocamelcontact(square ia)
{
  return noleapcontact(ia, vec_chameau_start, vec_chameau_end);
}

boolean noalfilcontact(square ia)
{
  return noleapcontact(ia, vec_alfil_start, vec_alfil_end);
}

boolean nodabbabacontact(square ia)
{
  return noleapcontact(ia, vec_dabbaba_start, vec_dabbaba_end);
}

boolean nozebracontact(square ia)
{
  return noleapcontact(ia, vec_zebre_start, vec_zebre_end);
}

boolean nogiraffecontact(square ia)
{
  return noleapcontact(ia, vec_girafe_start, vec_girafe_end);
}

boolean noantelopecontact(square ia)
{
  return noleapcontact(ia, vec_antilope_start, vec_antilope_end);
}


boolean nocontact(square sq_departure, square sq_arrival, square sq_capture, nocontactfunc_t nocontactfunc) {
  boolean   Result;
  square    cr;
  piece pj, pp, pren;
  piece pc= obs;
  square sq_castle_from=initsquare, sq_castle_to=initsquare;

  VARIABLE_INIT(cr);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  nextply();

  pj= e[sq_departure];
  pp= e[sq_capture];
  /* does this work with neutral pieces ??? */
  if (CondFlag[haanerchess]) {
    e[sq_departure]= obs;
  }
  else if (CondFlag[sentinelles]
           && !TSTFLAGMASK(sq_spec[sq_departure],BIT(WhPromSq)|BIT(BlPromSq))
           && !is_pawn(abs(pj)))
  {
    if ((pj<=roin) != SentPionAdverse) {
      if (number_of_pieces[Black][-sentinelle[Black]] < sentinelles_max_nr_pawns[Black]
          && number_of_pieces[White][sentinelle[White]]+number_of_pieces[Black][-sentinelle[Black]] < sentinelles_max_nr_pawns_total
          && (!flagparasent
              || (number_of_pieces[Black][-sentinelle[Black]]
                  <= number_of_pieces[White][sentinelle[White]]+(pp==sentinelle[White]?1:0))))
      {
        e[sq_departure]= sentinelle[Black];
      }
      else {
        e[sq_departure]= vide;
      }
    }
    else { /* we assume  pj >= roib */
      if (number_of_pieces[White][sentinelle[White]] < sentinelles_max_nr_pawns[White]
          && number_of_pieces[White][sentinelle[White]]+number_of_pieces[Black][-sentinelle[Black]] < sentinelles_max_nr_pawns_total
          && (!flagparasent
              || (number_of_pieces[White][sentinelle[White]]
                  <= number_of_pieces[Black][-sentinelle[Black]]+(pp==sentinelle[Black]?1:0))))
      {
        e[sq_departure]= sentinelle[White];
      }
      else {
        e[sq_departure]= vide;
      }
      /* don't think any change as a result of Sentinelles */
      /* PionNeutral is needed as piece specs not changed  */
    }
  }
  else {
    e[sq_departure]= vide;
    /* e[sq_departure] = CondFlag[haanerchess] ? obs : vide;       */
  }

  if (sq_capture == messigny_exchange) {
    e[sq_departure]= e[sq_arrival];
  }
  else
  {
    /* the pieces captured and reborn may be different: */
    /* Clone, Chameleon Circe               */
    pp= e[sq_capture];

    /* the pieces can be reborn at the square where it has been
     * captured. For example, when it is taken by a locust or a
     * similarly moving piece
     */
    e[sq_capture]= vide;

    if (anyparrain)
    {
      ply const grand_parent = parent_ply[parent_ply[nbply]];
      move_effect_journal_index_type const grand_parent_base = move_effect_journal_top[grand_parent-1];
      move_effect_journal_index_type const grand_parent_capture = grand_parent_base+move_effect_journal_index_offset_capture;
      if (move_effect_journal[grand_parent_capture].type==move_effect_piece_removal)
      {
        square const sq_capture = move_effect_journal[grand_parent_capture].u.piece_removal.from;
        if (CondFlag[parrain])
          cr = sq_capture + sq_arrival - sq_departure;
        if (CondFlag[contraparrain])
          cr = sq_capture - sq_arrival + sq_departure;
        pc = e[cr];
        if (pc==vide)
        {
          e[cr]= move_effect_journal[grand_parent_capture].u.piece_removal.removed;
          TraceSquare(cr);
          TraceText("\n");
        }
      }
    }

    if (pp != vide && pp != obs) {
      if (anycirce && abs(pp) > roib && !anyparrain) {
        /* This still doesn't work with neutral pieces.
        ** Eventually we must add the colour of the side making
        ** the move or potentially giving the check to the
        ** argument list!
        */
        if (anyclone && sq_departure != king_square[Black] && sq_departure != king_square[White]) {
          /* Circe Clone */
          pren = (pj * pp < 0) ? -pj : pj;
        }
        else {
          /* Chameleon Circe or ordinary Circe type */
          pren= CondFlag[chamcirce]
            ? (pp<vide ? -(piece)chameleon_circe_get_reborn_piece(abs(pp)) : (piece)chameleon_circe_get_reborn_piece(abs(pp)))
            : pp;
        }

        if (CondFlag[couscous])
          cr= (*circerenai)(abs(pj), spec[sq_departure], sq_capture, sq_departure, sq_arrival, advers(trait[nbply]));
        else
          cr= (*circerenai)(abs(pren), spec[sq_capture], sq_capture, sq_departure, sq_arrival, trait[nbply]);

        if ((pc= e[cr]) == vide) {
          e[cr]= pren;
        }
      } /* anycirce && abs(pp) > roib */
    } /* pp != vide && pp != obs */
    else { /* no capture move */
      if (abs(pj) == King)
      {
        if (castling_supported) {
              if (sq_capture == kingside_castling) {
            sq_castle_from = sq_arrival+dir_right;
            sq_castle_to = sq_arrival+dir_left;
              }
              else if (sq_capture == queenside_castling) {
            sq_castle_from = sq_arrival+2*dir_left;
            sq_castle_to = sq_arrival+dir_right;
              }
        }
        else if (CondFlag[castlingchess] && sq_capture > platzwechsel_rochade)
        {
          sq_castle_to = (sq_arrival + sq_departure) / 2;
          sq_castle_from = sq_capture - maxsquare;
        }
        else if (CondFlag[platzwechselrochade] && sq_capture == platzwechsel_rochade)
        {
          sq_castle_to = sq_arrival;
          sq_castle_from = sq_departure;
        }
        if (sq_castle_from != initsquare)
        {
          e[sq_castle_to]= e[sq_castle_from];
          e[sq_castle_from]= vide;
        }
      }
    }
  }

  if (CondFlag[contactgrid]) {
    Result= nogridcontact(sq_arrival);
  }
  else {
    Result= (*nocontactfunc)(sq_arrival);
  }

  if (pc != obs) {
    e[cr]= pc;
  }

  e[sq_capture]= pp;
  e[sq_departure]= pj;
  if (sq_castle_from != initsquare) {
      e[sq_castle_from]= e[sq_castle_to];
    e[sq_castle_to] = vide;
  }
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",Result);
  TraceFunctionResultEnd();
  return Result;
} /* nocontact */

static void locate_observees(PieNam walk, square pos_observees[])
{
  unsigned int current = 0;
  square const *bnp;

  for (bnp = boardnum; current<number_of_pieces[trait[nbply]][walk]; ++bnp)
    if (abs(e[*bnp])==walk && TSTFLAG(spec[*bnp],trait[nbply]))
    {
      pos_observees[current] = *bnp;
      ++current;
    }
}

static void isolate_observee(PieNam walk, square const pos_observees[], unsigned int isolated_observee)
{
  unsigned int orphan_id;

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][walk]; ++orphan_id)
    if (orphan_id!=isolated_observee)
      e[pos_observees[orphan_id]] = dummyb;
}

static void restore_observees(PieNam walk, square const pos_observees[])
{
  unsigned int orphan_id;
  piece const observee_type = trait[nbply]==White ? walk : -walk;

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][walk]; ++orphan_id)
    e[pos_observees[orphan_id]] = observee_type;
}

static boolean find_next_orphan_in_chain(square sq_target,
                                         square const pos_orphans[],
                                         PieNam orphan_observer,
                                         evalfunction_t *evaluate)
{
  boolean result = false;
  unsigned int orphan_id;

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][Orphan]; ++orphan_id)
  {
    boolean does_orphan_observe;

    isolate_observee(Orphan,pos_orphans,orphan_id);
    does_orphan_observe = (*checkfunctions[orphan_observer])(sq_target,
                                                             Orphan,
                                                             evaluate);
    restore_observees(Orphan,pos_orphans);

    if (does_orphan_observe
        && orphan_find_observation_chain(pos_orphans[orphan_id],
                                         orphan_observer,
                                         evaluate))
    {
      result = true;
      break;
    }
  }

  return result;
}

boolean orphan_find_observation_chain(square sq_target,
                                      PieNam orphan_observer,
                                      evalfunction_t *evaluate)
{
  boolean result;

  trait[nbply] = advers(trait[nbply]);

  if ((*checkfunctions[orphan_observer])(sq_target,orphan_observer,evaluate))
    result = true;
  else if (number_of_pieces[trait[nbply]][Orphan]==0)
    result = false;
  else
  {
    piece const orphan_type = e[sq_target];
    --number_of_pieces[advers(trait[nbply])][Orphan];
    e[sq_target] = dummyb;

    {
      square pos_orphans[63];
      locate_observees(Orphan,pos_orphans);
      result = find_next_orphan_in_chain(sq_target,
                                         pos_orphans,
                                         orphan_observer,
                                         evaluate);
    }

    e[sq_target] = orphan_type;
    ++number_of_pieces[advers(trait[nbply])][Orphan];
  }

  trait[nbply] = advers(trait[nbply]);

  return result;
}

boolean orphancheck(square sq_target,
                    PieNam orphan_type,
                    evalfunction_t *evaluate)
{
  boolean result = false;
  PieNam const *orphan_observer;
  square pos_orphans[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(orphan_type);
  TraceFunctionParamListEnd();

  locate_observees(Orphan,pos_orphans);

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; orphan_observer++)
    if (number_of_pieces[White][*orphan_observer]+number_of_pieces[Black][*orphan_observer]>0
        && find_next_orphan_in_chain(sq_target,
                                     pos_orphans,
                                     *orphan_observer,
                                     evaluate))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  return result;
}

boolean find_next_friend_in_chain(square sq_target,
                                  PieNam friend_observer,
                                  PieNam friend_type,
                                  evalfunction_t *evaluate)
{
  boolean result = false;

  if ((*checkfunctions[friend_observer])(sq_target,friend_observer,evaluate))
    result = true;
  else
  {
    --number_of_pieces[trait[nbply]][Friend];

    if (number_of_pieces[trait[nbply]][friend_type]>0)
    {
      unsigned int k;
      square pos_remaining_friends[63];
      piece const save_friend = e[sq_target];

      e[sq_target] = dummyb;

      locate_observees(Friend,pos_remaining_friends);

      for (k = 0; k<number_of_pieces[trait[nbply]][friend_type]; k++)
      {
        boolean is_friend_observed;

        isolate_observee(Friend,pos_remaining_friends,k);
        is_friend_observed = (*checkfunctions[friend_observer])(sq_target,Friend,evaluate);
        restore_observees(Friend,pos_remaining_friends);

        if (is_friend_observed
            && find_next_friend_in_chain(pos_remaining_friends[k],friend_observer,friend_type,evaluate))
        {
          result = true;
          break;
        }
      }

      e[sq_target] = save_friend;
    }

    ++number_of_pieces[trait[nbply]][Friend];
  }

  return result;
}

boolean friendcheck(square sq_king,
                    PieNam p,
                    evalfunction_t *evaluate)
{
  PieNam const *pfr;
  boolean result = false;
  square pos_friends[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TracePiece(p);
  TraceFunctionParamListEnd();

  locate_observees(Friend,pos_friends);

  for (pfr = orphanpieces; *pfr!=Empty; pfr++)
    if (number_of_pieces[trait[nbply]][*pfr]>0)
    {
      unsigned int k;
      for (k = 0; k<number_of_pieces[trait[nbply]][Friend]; ++k)
      {
        boolean does_friend_observe;

        isolate_observee(Friend,pos_friends,k);
        does_friend_observe = (*checkfunctions[*pfr])(sq_king,Friend,evaluate);
        restore_observees(Friend,pos_friends);

        if (does_friend_observe
            && find_next_friend_in_chain(pos_friends[k],*pfr,p,evaluate))
        {
          result = true;
          break;
        }
      }

      if (result)
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean CrossesGridLines(square dep, square arr)
{
  int i, x1, y1, x2, y2, X1, Y1, X2, Y2, dx, dy, dX, dY, u1, u2, v;

  X1= ((dep<<1) -15) % 24;
  Y1= ((dep/24)<<1) - 15;
  X2= ((arr<<1) -15) % 24;
  Y2= ((arr/24)<<1) - 15;
  dX= X2-X1;
  dY= Y2-Y1;
  for (i= 0; i < numgridlines; i++)
  {
    x1= gridlines[i][0];
    y1= gridlines[i][1];
    x2= gridlines[i][2];
    y2= gridlines[i][3];
    dx= x2-x1;
    dy= y2-y1;
    v=dY*dx-dX*dy;
    if (!v)
      continue;
    u1= dX*(y1-Y1)-dY*(x1-X1);
    if (v<0? (u1>0 || u1<v) : (u1<0 || u1>v))
      continue;
    u2= dx*(y1-Y1)-dy*(x1-X1);
    if (v<0? (u2>0 || u2<v) : (u2<0 || u2>v))
      continue;
    return true;
  }
  return false;
}
