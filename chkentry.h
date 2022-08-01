/* vim: set tabstop=8 softtabstop=4 shiftwidth=4 noexpandtab : */
/*
 * chkentry - check JSON files in an IOCCC entry
 *
 * "Because grammar and syntax alone do not make a complete language." :-)
 *
 * The JSON parser was co-developed by:
 *
 *	@xexyl
 *	https://xexyl.net		Cody Boone Ferguson
 *	https://ioccc.xexyl.net
 * and:
 *	chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * This tool is being co-developed by Cody Boone Ferguson and Landon Curt Noll
 * and the concept of this file was developed by Landon Curt Noll.
 *
 * "Because sometimes even the IOCCC Judges need some help." :-)
 */


#if !defined(INCLUDE_CHKENTRY_H)
#    define  INCLUDE_CHKENTRY_H


/*
 * dbg - info, debug, warning, error, and usage message facility
 */
#include "dbg.h"

/*
 * json_util - general JSON parser utility support functions
 */
#include "json_util.h"

/*
 * util - entry common utility functions for the IOCCC toolkit
 */
#include "util.h"

/*
 * time - for time specific things
 */
#include <time.h>

/*
 * non-strict match to 1 part in MATCH_PRECISION
 */
#define MATCH_PRECISION ((long double)(1<<22))


/*
 * usage message
 *
 * Use the usage() function to print the usage_msg([0-9]?)+ strings.
 */
static const char * const usage_msg =
    "usage: %s [-h] [-v level] [-J level] [-V] [-q] [-F fnamchk] entry_dir\n"
    "usage: %s [-h] [-v level] [-J level] [-V] [-q] [-F fnamchk] info.json author.json\n"
    "\n"
    "\t-h\t\tprint help message and exit 0\n"
    "\t-v level\tset verbosity level (def level: %d)\n"
    "\t-J level\tset JSON verbosity level (def level: %d)\n"
    "\t-V\t\tprint version string and exit 3\n"
    "\t-q\t\tquiet mode (def: not quiet)\n"
    "\t\t\t    NOTE: -q will also silence msg(), warn(), warnp() if -v 0\n"
    "\t-F fnamchk\tpath to fnamchk tool (def: %s)\n"
    "\n"
    "entry_dir\tIOCCC entry directory with .info.json and author.json files\n"
    "info.json\tcheck info.json file, . ==> skip IOCCC info check\n"
    "author.json\tcheck author.json file, . ==> skip IOCCC author check\n"
    "\n"
    "Exit codes:\n"
    "    0\tall is OK\n"
    "    3\t-h and help string printed or -V and version string printed\n"
    "    4\tcommand line error\n"
    "    >=5\tinternal error\n"
    "\n"
    "chkentry version: %s\n";


/*
 * globals
 */
bool quiet = false;				/* true ==> quiet mode */


/*
 * externals
 */

/*
 * function prototypes
 */
static void usage(int exitcode, char const *prog, char const *str, int expected, int argc) __attribute__((noreturn));


/* Special stuff you're not supposed to know about. :-( */
static void Bfgrerv(int d, int y);
static int D = -1, Y = -1;
static time_t t;
static char const *Bfgrexbeo[] =
{
    "'yrff' vf yrkvpbtencuvpnyyl terngre guna 'terngre'. '<', ubjrire,\n"
    "vf yrkvpbtencuvpnyyl yrff guna '>'. Fbzrbar fperjrq hc.\n-- Krkly",
    "V qba'g xabj unys bs lbh unys nf jryy nf V fubhyq yvxr;\nnaq V yvxr yrff guna"
    " unys bs lbh unys nf jryy nf lbh qrfreir.\n-- Ovyob Onttvaf",
    "P frpgvba: '{}'\n-- Krkly\n",
    "'Frpbaqyl, gb pryroengr BHE oveguqnlf: zvar naq zl ubabhenoyr naq tnyynag\n"
    "sngure'f.' Hapbzsbegnoyr naq nccerurafvir fvyrapr. 'V nz bayl unys gur zna\n"
    "gung ur vf: V nz 72, ur vf 144. Lbhe ahzoref ner pubfra gb qb ubabhe gb\n"
    "rnpu bs uvf ubabhenoyr lrnef.' Guvf jnf ernyyl qernqshy -- n erthyne\n"
    "oenvagjvfgre, naq fbzr bs gurz sryg vafhygrq, yvxr yrnc-qnlf fubirq\n"
    "va gb svyy hc n pnyraqne.\n-- Ovatb Onttvaf",
    "P havbaf: ybivat pbhcyrf jub rawbl cebtenzzvat va P gbtrgure.\n-- Krkly\n",
    "'Ybbx,' fnvq Neguhe, 'jbhyq vg fnir lbh n ybg bs gvzr vs V whfg tnir\n"
    "hc naq jrag znq abj?'\n",
    "Gurer vf na neg, be, engure, n xanpx gb sylvat. Gur xanpx yvrf va\n"
    "yrneavat ubj gb guebj lbhefrys ng gur tebhaq naq zvff.\n",
    "SHA SNPG: Va O naq rneyl P, gur += bcrengbe jnf fcryg =+ vafgrnq bs += ;\n"
    "guvf zvfgnxr, ercnverq va 1976, jnf vaqhprq ol n frqhpgviryl rnfl jnl bs\n"
    "unaqyvat gur svefg sbez va O'f yrkvpny nanylfre.\n\nFrr zber sha snpgf naq"
    " uvfgbel ol Qraavf Evgpuvr uvzfrys ng\nuggcf://jjj.oryy-ynof.pbz/hfe/qze/jjj/puvfg.ugzy.\n",
    "'Jung vf vg?' tebjyrq Fnz, zvfvagrecergvat gur fvtaf. 'Jung'f gur arrq gb\n"
    "favss? Gur fgvax arneyl xabpxf zr qbja jvgu zl abfr uryq. Lbh fgvax, naq znfgre\n"
    "fgvaxf; gur jubyr cynpr fgvaxf.'\n\n'Lrf, lrf, naq Fnz fgvaxf!' nafjrerq Tbyyhz. "
    "'Cbbe Fzéntby fzryyf vg,\nohg tbbq Fzéntby ornef vg. Urycf avpr znfgre. Ohg gung'f"
    " ab znggre. Gur nve'f\nzbivat, punatr vf pbzvat. Fzéntby jbaqref; ur'f abg unccl.'\n",
    "'Anl! Abg Ryirf,' fnvq gur sbhegu, gur gnyyrfg, naq nf vg nccrnerq gur\npuvrs nzbat"
    " gurz. 'Ryirf qb abg jnyx va Vguvyvra va gurfr qnlf. Naq\nRyirf ner jbaqebhf snve gb"
    " ybbx hcba, be fb 'gvf fnvq.'\n\n'Zrnavat jr'er abg, V gnxr lbh,' fnvq Fnz. 'Gunax"
    " lbh xvaqyl. Naq jura\nlbh'ir svavfurq qvfphffvat hf, creuncf lbh'yy fnl jub lbh ner,"
    " naq\njul lbh pna'g yrg gjb gverq geniryyref erfg.'\n",
    "Tvyqbe jnf fvyrag sbe n zbzrag. 'V qb abg yvxr guvf arjf,' ur fnvq\n"
    "ng ynfg. 'Gung Tnaqnys fubhyq or yngr, qbrf abg obqr jryy. Ohg vg\nvf fnvq:"
    " Qb abg zrqqyr va gur nssnvef bs Jvmneqf, sbe gurl ner\nfhogyr naq dhvpx gb "
    "natre. Gur pubvpr vf lbhef: gb tb be jnvg.'\n\n'Naq vg vf nyfb fnvq,' nafjrerq"
    " Sebqb: 'Tb abg gb gur Ryirf sbe\npbhafry, sbe gurl jvyy fnl obgu ab naq lrf.'",
    "Bar bs gur guvatf Sbeq Cersrpg unq nyjnlf sbhaq uneqrfg gb haqrefgnaq nobhg\n"
    "uhznaf jnf gurve unovg bs pbagvahnyyl fgngvat naq ercrngvat gur irel irel\n"
    "boivbhf.\n\nBar bs gur guvatf Sbeq Cersrpg unq nyjnlf sbhaq uneqrfg gb haqrefgnaq nobhg\n"
    "uhznaf jnf gurve unovg bs pbagvahnyyl fgngvat naq ercrngvat gur irel irel\nboivbhf.",
    "Sbe n zbzrag, abguvat unccrarq. Gura, nsgre n frpbaq be fb, abguvat\npbagvahrq gb unccra.",
    "'Vs V rire zrrg zlfrys,' fnvq Mncubq, 'V'yy uvg zlfrys fb uneq V jba'g xabj\njung'f uvg zr.'",
    "'V frr abobql ba gur ebnq,' fnvq Nyvpr.\n\n'V bayl jvfu gung V unq fhpu rlrf,' gur Xvat erznexrq"
    " va n sergshy\ngbar. 'Gb or noyr gb frr Abobql! Naq ng gung qvfgnapr gbb! Jul, vg'f\nnf zhpu nf V"
    " pna qb gb frr erny crbcyr, ol guvf yvtug!'\n\n[...]\n\n'Jub qvq lbh cnff ba gur ebnq?' gur Xvat jrag"
    " ba, ubyqvat bhg uvf\nunaq gb gur Zrffratre sbe fbzr zber unl.\n\n'Abobql,' fnvq gur Zrffratre.\n\n'"
    "Dhvgr evtug,' fnvq gur Xvat: 'guvf lbhat ynql fnj uvz gbb. Fb bs pbhefr Abobql\njnyxf fybjre guna lbh.'\n\n"
    "'V qb zl orfg,' gur Zrffratre fnvq va n fhyxl gbar. 'V'z fher abobql jnyxf zhpu\nsnfgre guna V qb!'\n\n"
    "'Ur pna'g qb gung,' fnvq gur Xvat, 'be ryfr ur'q unir orra urer svefg.'",
    "'Guvf vf n puvyq!' Unvtun ercyvrq rntreyl, pbzvat va sebag bs Nyvpr gb\n"
    "vagebqhpr ure, naq fcernqvat bhg obgu uvf unaqf gbjneqf ure va na Natyb-Fnkba\n"
    "nggvghqr. 'Jr bayl sbhaq vg gb-qnl. Vg'f nf ynetr nf yvsr, naq gjvpr nf\nangheny!'\n\n"
    "'V nyjnlf gubhtug gurl jrer snohybhf zbafgref!' fnvq gur Havpbea. 'Vf vg\nnyvir?'\n\n'"
    "Vg pna gnyx,' fnvq Unvtun, fbyrzayl.\nGur Havpbea ybbxrq qernzvyl ng Nyvpr, naq fnvq 'Gnyx, puvyq.'\n"
    "Nyvpr pbhyq abg uryc ure yvcf pheyvat hc vagb n fzvyr nf fur ortna: 'Qb lbh\nxabj, V nyjnlf gubhtug"
    " Havpbeaf jrer snohybhf zbafgref, gbb! V arire fnj bar\nnyvir orsber!'\n\n[...]\n\n'Nu, jung vf vg, abj?'"
    " gur Havpbea pevrq rntreyl. 'Lbh'yy arire thrff! V\npbhyqa'g.' Gur Yvba ybbxrq ng Nyvpr jrnevyl. 'Ner lbh"
    " na navzny -- irtrgnoyr --\nbe zvareny?' ur fnvq, lnjavat ng rirel bgure jbeq.\n\n'Vg'f n snohybhf zbafgre!'"
    " gur Havpbea pevrq bhg, orsber Nyvpr pbhyq ercyl.\n\n'Gura unaq ebhaq gur cyhz-pnxr, Zbafgre,' gur Yvba fnvq.",
    "V ortna gb unir qbhogf nobhg ubj zngu vf gnhtug va guvf pbhagel jura n lbhat\n"
    "pbbx ng zl cvmmrevn fnvq ur unq n ceboyrz. 'N phfgbzre pnyyrq, nfxvat sbe n\ncvr gung'f n"
    " guveq purrfr, n guveq crccrebav, naq n guveq pbzovangvba,' ur\nfnvq. 'Fb, jung'f gur ceboyrz?' "
    "V nfxrq uvz. 'V qba'g xabj jung gb chg ba\ngur sbhegu guveq'.\n\n-EBOREG XRNEARL\nEnapub Pbeqbin,\nPnyvsbeavn",
    "'Naq lbh qb Nqqvgvba?' gur Juvgr Dhrra nfxrq. 'Jung'f bar naq bar naq bar naq\nbar naq bar naq bar"
    " naq bar naq bar naq bar naq bar?' 'V qba'g xabj,' fnvq\nNyvpr. 'V ybfg pbhag.'\n\n'Fur pna'g qb Nqqvgvba,'"
    " gur Erq Dhrra vagreehcgrq. 'Pna lbh qb Fhogenpgvba?'\nGnxr avar sebz rvtug.' 'Avar sebz rvtug V pna'g, lbh xabj,'"
    " Nyvpr ercyvrq irel\nernqvyl: 'ohg--'\n\n'Fur pna'g qb Fhogenpgvba,' fnvq gur Juvgr Dhrra. 'Pna lbh qb Qvivfvba? "
    "Qvivqr\nn ybns ol n xavsr -- jung'f gur nafjre gb gung?' 'V fhccbfr--' Nyvpr jnf\nortvaavat, ohg gur Erq Dhrra "
    "nafjrerq sbe ure.  'Oernq-naq-ohggre, bs pbhefr.\nGel nabgure Fhogenpgvba fhz. Gnxr n obar sebz n qbt: jung erznvaf?'\n\n"
    "Nyvpr pbafvqrerq. 'Gur obar jbhyqa'g erznva, bs pbhefr, vs V gbbx vg--naq gur\nqbt jbhyqa'g erznva; vg jbhyq pbzr gb ovgr "
    "zr--naq V'z fher V fubhyqa'g\nerznva!' 'Gura lbh guvax abguvat jbhyq erznva?' fnvq gur Erq Dhrra.\n\n'V guvax gung'f gur "
    "nafjre.' 'Jebat, nf hfhny,' fnvq gur Erq Dhrra: 'gur qbt'f\ngrzcre jbhyq erznva.' 'Ohg V qba'g frr ubj --'\n\n"
    "'Jul, ybbx urer!' gur Erq Dhrra pevrq. 'Gur qbt jbhyq ybfr vgf grzcre, jbhyqa'g\nvg?'\n\n"
    "'Creuncf vg jbhyq,' Nyvpr ercyvrq pnhgvbhfyl. 'Gura vs gur qbt jrag njnl, vgf\ngrzcre jbhyq erznva!' "
    "gur Dhrra rkpynvzrq gevhzcunagyl.",
    "Urer gur Erq Dhrra ortna ntnva. 'Pna lbh nafjre hfrshy dhrfgvbaf?' fur fnvq.\n"
    "'Ubj vf oernq znqr?'\n\n'V xabj gung!' Nyvpr pevrq rntreyl.' Lbh gnxr fbzr sybhe--'\n"
    "'Jurer qb lbh cvpx gur sybjre?' gur Juvgr Dhrra nfxrq. 'Va n tneqra, be va gur\nurqtrf?'\n\n"
    "'Jryy vg vfa'g cvpxrq ng nyy,' Nyvpr rkcynvarq: 'vg'f tebhaq--'\n'Ubj znal nperf bs tebhaq?' "
    "fnvq gur Juvgr Dhrra. 'Lbh zhfga'g yrnir bhg fb\nznal guvatf.'\n\n'Sna ure urnq!' gur Erq Dhrra "
    "nakvbhfyl vagreehcgrq. 'Fur'yy or srirevfu nsgre\nfb zhpu guvaxvat.' Fb gurl frg gb jbex naq snaarq "
    "ure jvgu ohapurf bs yrnirf,\ngvyy fur unq gb ort gurz gb yrnir bss, vg oyrj ure unve nobhg fb.",
    NULL
};

#endif /* INCLUDE_CHKENTRY_H */
