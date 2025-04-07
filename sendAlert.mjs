export function sendAlert()
{
    var distOk = (distanceToMe < 50000) || ((distanceToMe < 100000) && (gettingCloser < 20))
	var lfbd = (distanceToMe < 30000) && (altitude < 10000) && (groundSpeed > 100)
    var flagsOk = dbFlags.includes("MIL")
    var csOk = ["ZEROG"].includes(callsign) || callsign.startsWith("AIB") || callsign.startsWith("MEDOC")
    var codeOk = ["A400","A3ST","A337","DH8D","AT8T","A139"].includes(typeCode) || typeCode.startsWith("AN")
    var codeNOK = ["TBM7","EC45","PC21","PC6T","AS55","EC35","B350"].includes(typeCode)
    var nbPropOK = parseInt(typeDesc.substr(1,1)) > 2
	var squawkOk = ["7700","7600","7500"].includes(squawk)
    var icaoUSA = hex.startsWith("A") && !(typeDesc=="L1P")
	var lowlevel = (altitude < 20000)
	var bigplane = (cat=="A5")
	var privateplane = dbFlags.includes("LADD")
	var unk = ["Unknown"].includes(country)
	
    /*
    console.log("distance", distOk);
    console.log("flags", flagsOk);
    console.log("callsign", csOk);
    console.log("typeCode", codeOk);
    console.log("!typeCode", codeNOK);
    console.log("nbPropOK", nbPropOK);
    */    

    /*
    return distOk                     // distance ok
        && ((flagsOk && !codeNOK)     // military filtered from common aircrafts
            || csOk                   // interesting callsign
            || codeOk                 // interesting typeCode
            || nbPropOK               // aircraft with more than 2 reactors
			|| squawkOk               // emergency
			|| icaoUSA                // US plane except small props
			|| unk                    // unknown plane
			|| (lowlevel   
			    && (bigplane
                    || privateplane	)))
	*/
	
	if(distOk){	    
		if (squawkOk) return 1
	    if (flagsOk && !codeNOK) return 2
		if (lowlevel && (bigplane || privateplane)) return 3
		if (codeOk || nbPropOK) return 4
		if (csOk || icaoUSA || unk) return 10
	}
	
	return 0
}
