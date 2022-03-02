export function sendAlert()
{
    var distOk = (distanceToMe < 50000) || ((distanceToMe < 100000) && (gettingCloser < 20))
    var flagsOk = dbFlags.includes("MIL")
    var csOk = ["ZEROG"].includes(callsign)
    var codeOk = ["A400","A3ST","A337"].includes(typeCode) || typeCode.startsWith("AN")
    var codeNOK = ["TBM7","EC45","PC21","PC6T","AS55","EC35","B350"].includes(typeCode)
    var nbPropOK = parseInt(typeDesc.substr(1,1)) > 2

    /*
    console.log("distance", distOk);
    console.log("flags", flagsOk);
    console.log("callsign", csOk);
    console.log("typeCode", codeOk);
    console.log("!typeCode", codeNOK);
    console.log("nbPropOK", nbPropOK);
    */

    return distOk                     // distance ok
        && ((flagsOk && !codeNOK)     // military filtered from common aircrafts
            || csOk                   // interesting callsign
            || codeOk                 // interesting typeCode
            || nbPropOK)               // aircraft with more than 2 reactors
}
