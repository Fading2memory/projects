
Eductional Purposes only!

Not the full source material for dreadmistfall, aka dreadmist.
But proof that I can program metamorphic code.


Dreadmistfall includes ~

1.) Infect.inc --- this is our file infector, plus our MBR infector it will save the virus to the MBR at an offset. It is then my job
  to find that offset in which the virus is saved too, and clear it so it does not spread. 

2.) Permut.inc --- the polymorphic/metamorphic engine in which the virus generates new copies of itself upon each new infection. 
  It is greatly difficult to spot this without heuristics analysis, but each and every day virus/malware creators finds new ways to 
  overcome and beat heuristics analysis.

 3.) Shrinker & Xpander.inc --- The shrinker acts as a compression engine, and the xpander (aka expander) is only called to either make 
  the code size either bigger or smaller on each generation (meaning, on each new infection, the virus size is either bigger or smaller).
