#if !defined(CONDITIONS_PHANTOM_H)
#define CONDITIONS_PHANTOM_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* This module provides implements the condition Phantom Chess
 */

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type phantom_en_passant_adjuster_attack(slice_index si,
                                                    stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type phantom_en_passant_adjuster_defend(slice_index si,
                                                    stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_phantom_en_passant_adjusters(slice_index si);

#endif
