
#include "src.hpp"
#include <iostream>
using namespace std;
int main(){
    pylist ls; ls.append(1); cout<<ls[0]<<"\n"; ls.append(2); cout<<ls[1]<<"\n"; ls.pop(); ls.append(3); cout<<ls[1]<<"\n";
    pylist ls2 = ls; ls.append(4); cout<<ls2[2]<<"\n"; ls.pop(); ls2.append(5); cout<<ls[2]<<"\n";
    pylist ls3; ls3.append(6); ls[0]=ls3; cout<<ls2[0][0]<<"\n"; ls2.append(ls3); ls2[3].append(7); ls[1]=pylist(); cout<<ls[0][1]<<"\n";
    ls2[2] = ls[1]; ls2[1].append(8); cout<<ls2[2][0]<<"\n";
    ls[0] = pylist(); ls2 = ls; ls3 = ls3; ls2 = ls3; ls3[0] = ls3; ls2.append(3*3); cout<<ls[3][0][0][0][0][0][0][0][0][0][0][0][0][2]<<"\n";
    cout << 2 * ls[3][0][2] + (ls2[1] << 1 >> 1) - (1 ^ ls2[2] ^ 1) - (ls2[1] / 1) - (ls[1][0] % 8) + 1 << endl;
    cout<<ls<<"\n"; cout<<ls.pop()<<"\n"; cout<<0 + ls[2].pop()<<"\n"; cout<<ls<<"\n";
}
