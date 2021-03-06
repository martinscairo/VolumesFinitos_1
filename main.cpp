//==============================================================================
// Name        : main.cpp
// Author      : Cairo Martins e Leonardo Thimoteo
// Version     : 1.0
// Description : Program to develop the finite volumes method
// Status      : Needs verification
//==============================================================================


//==============================================================================
//                              C++ Includes 
//==============================================================================
#include <iostream>
#include <vector>
#include <string>

//==============================================================================
//                               Own includes 
//==============================================================================
#include <Volumes.h>

//==============================================================================
//                               Other includes 
//==============================================================================
#include <libconfig.h++>

//==============================================================================
//                                 typedef
//==============================================================================]
typedef std :: vector <std::string>                   vecString;
typedef std :: vector <Real>                          vecReal;


//==============================================================================
//                                prototypes
//==============================================================================
const int LeituraDadosProblema (Volumes&);

void GeracaoMalha (vecReal&, vecReal&, vecReal&, vecReal&, Volumes&);

void CalculaCoeficientes (vecReal&, vecReal&, vecReal&, vecReal&,
                          vecReal&, vecReal&);



//==============================================================================
//                              main function
//==============================================================================

int main(int argc, char** argv)
{  
    
//------------------------------------------------------------------------------
//                    Assimilando dados do arquivo externo
//------------------------------------------------------------------------------
    Volumes v1;    
    LeituraDadosProblema (v1);
    
    std :: vector <Real>      xFronteiras (v1.NVOL()+1), //localizações fronteiras
                              xCentro     (v1.NVOL()),   //localizações centros
                              DistCentro  (v1.NVOL()+1), //distâncias entre centros
                              DistFace    (v1.NVOL());   //distância entre fronteiras adjacentes
   

    GeracaoMalha (xFronteiras, xCentro, DistCentro, DistFace, v1);
    
    std :: vector <Real>      Ap(v1.NVOL()),
                              Ae(v1.NVOL()),
                              Aw(v1.NVOL()),
                              Sp(v1.NVOL());
    
    CalculaCoeficientes (DistFace, DistCentro, Ap, Ae, Aw, Sp);
    
    for (int i=0; i<v1.NVOL(); i++)
    {
    std :: cout << "Ae[" << i << "]=" << Ap[i] << std :: endl;
    } 
    
}    
//==============================================================================
//                           funções auxiliares
//==============================================================================

//------------------------------------------------------------------------------
//                  1. Função para ler dados dos problemas
//------------------------------------------------------------------------------
    
 const int LeituraDadosProblema (Volumes& _vol)
 {
     libconfig :: Config        cfg_txt;
     const std :: string        FILENAME("Volumes.txt");
     
     try
     {
         cfg_txt.readFile (FILENAME.c_str());         
     }
     
     catch (const libconfig :: FileIOException &fioex)
     {
         std :: cerr << "Arquivo "
                     << FILENAME
                     << " não foi encontrado. \n"
                     << "Execução cancelada. \n";
         return (EXIT_FAILURE);
     }
     
     
     catch (const libconfig :: ParseException &pex)
     {
         std :: cerr << "Erro no arquivo: " << pex.getFile()
                     << "\nNa linha: "      << pex.getLine()
                     << "\nErro: "          << pex.getError() 
                     << std :: endl;
         
         return (EXIT_FAILURE);
     }
     
 const vecString                      vecDados = {"Comprimento",
                                                  "Abscissa_inicial",
                                                  "Imagem_esquerda",
                                                  "Imagem_direita",
                                                  "Numero_Volumes"
                                                  };

     
     try
     {
         for (auto &ItVecDados : vecDados)
         {
             cfg_txt.lookup(ItVecDados);
         }
     }  
     
         catch (const libconfig :: SettingNotFoundException &nfex)
         {
             std :: cerr << "Parâmetro não encontrado: " 
                         << nfex.getPath() << "\n" 
                         << nfex.what() << std :: endl;
         }
         
     
     _vol.comprimento    = cfg_txt.lookup(vecDados[0]);
     _vol.xFace          = cfg_txt.lookup(vecDados[1]);
     _vol.imagemEsquerda = cfg_txt.lookup(vecDados[2]);
     _vol.imagemDireita  = cfg_txt.lookup(vecDados[3]);     
     _vol.nVol           = cfg_txt.lookup(vecDados[4]);
     
          
     std :: cout << "Comprimento do domínio: "       << _vol.comprimento
                 << "\nNúmero de Volumes na malha: " << _vol.nVol
                 << "\nAbscissa da face inicial: "   << _vol.xFace
                 <<"\nImagem à esquerda: "           << _vol.imagemEsquerda
                 << "\nImagem à direita: "            << _vol.imagemDireita
                 << std :: endl;
 }
 
 
 
 
void GeracaoMalha(vecReal& xfronteira, vecReal& xcentro, vecReal& distcentro,
                  vecReal& distface, Volumes& _vol) 
 {
     Real DistFronteiras(_vol.comprimento/_vol.nVol), 
          DistCentros (DistFronteiras);
     
 //___________________1. Completando vetores de fronteiras______________________
     
     for (int i=0; i<_vol.NVOL() +1; i++)
     {
        if (i==0) xfronteira[0] = _vol.XFACE();
        
        else xfronteira[i] = xfronteira[i-1] + DistFronteiras;
     }
   
//_______________2.Completando vetores coordenadas do centro____________________
     
     for (int i=0; i<_vol.NVOL(); i++)
     {
            xcentro[i] = (xfronteira[i+1] +xfronteira[i])*0.5;
     }
     
//_____________3. Completando vetores com distâncias entre centros______________
     
    for (int i=0; i<_vol.NVOL()+1; i++) 
    {
        if (i==0)  distcentro[i] = xcentro[i] - xfronteira[i];
        
        else if(i==_vol.NVOL()) distcentro[i] = xfronteira[i] - xcentro[i-1];
        
        else distcentro[i] = xcentro[i]- xcentro[i-1];
      
    }
     
     
//______________4.Completando vetor distância entre fronteiras__________________
     
     for (int i=0; i<_vol.NVOL(); i++)
     {
         distface[i] = xfronteira[i+1] - xfronteira[i];
     }    
}

 

void CalculaCoeficientes (vecReal& distf, vecReal& distc, vecReal& ap, 
                          vecReal& ae, vecReal& aw, vecReal& sp)
{
    
//__________________1.Completando vetor diagonal oeste__________________________
    for (int i=0; i<distf.size(); i++)
    {
       if (i==0) aw[i] = 0;
       else   aw[i] = -1/distc[i];
    }
 
//__________________2.Completando vetor diagonal leste__________________________
   for (int i=0; i<distf.size(); i++)
   {
       if (i==distf.size()-1) ae[i]=0;
       else ae[i] = -1/distc[i+1];
   }
  
//__________________1.Completando vetor diagonal principal______________________
    for (int i=0; i<distf.size(); i++)
    {
        ap[i] = (1/distc[i+1]) + (1/distc[i]);
    }   
    
}