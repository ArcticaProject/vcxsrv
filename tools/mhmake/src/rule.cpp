/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2009 Marc Haesen
 *
 *  Mhmake is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mhmake is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mhmake.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Rev$ */

#include "stdafx.h"

#include "fileinfo.h"
#include "rule.h"
#include "util.h"
#include "mhmakeparser.h"

refptr<rule> NullRule;

map< string, vector<pair<string,refptr<rule> > > > IMPLICITRULE::m_ImplicitRules;

makecommand g_MakeCommand;  // Order is important since sm_Statics is using g_MakeCommand
const string g_QuoteString("\""); // Order is important since sm_Statics is using g_QuoteString
loadedmakefile::loadedmakefile_statics loadedmakefile::sm_Statics;

///////////////////////////////////////////////////////////////////////////////
static bool FindDep(const string &DepName,vector<pair<string,refptr<rule> > >&ImplicitRule,refptr<rule> &Rule)
{
  vector<pair<string,refptr<rule> > >::iterator SecIt=ImplicitRule.begin();
  while (SecIt!=ImplicitRule.end())
  {
    if (SecIt->first==DepName)
    {
      #ifdef _DEBUG
      // Check if the rule has the same commands
      vector<string> &OldCommands=SecIt->second->GetCommands();
      vector<string> &NewCommands=Rule->GetCommands();

      bool bCommandsDifferent=OldCommands.size()!=NewCommands.size();
      if (g_PrintMultipleDefinedRules || bCommandsDifferent)
      {
        string ErrorMessage;
        if (bCommandsDifferent)
          ErrorMessage += "Implicit Rule '"+ DepName + "' defined twice with different commands\n";
        else
          ErrorMessage += "Implicit Rule '"+ DepName + "' defined twice with same commands\n";
        ErrorMessage += "Command 1: makedir = " + SecIt->second->GetMakefile()->GetMakeDir()->GetQuotedFullFileName()+ "\n";

        vector<string>::const_iterator It;
        if (bCommandsDifferent)
        {
          It=OldCommands.begin();
          while (It!=OldCommands.end())
          {
            ErrorMessage += "  " + *It + "\n";
          }
        }
        cerr << "Command 2: makedir = "+ Rule->GetMakefile()->GetMakeDir()->GetQuotedFullFileName()+ "\n";
        if (bCommandsDifferent)
        {
          It=NewCommands.begin();
          while (It!=NewCommands.end())
          {
            ErrorMessage += "  " + *It + "\n";
          }
          throw ErrorMessage;
        }
        else
          cerr << ErrorMessage << endl;
      }
      mhmakeparser *pOldMakefile=SecIt->second->GetMakefile();
      mhmakeparser *pNewMakefile=Rule->GetMakefile();
      vector<string>::iterator OldIt=OldCommands.begin();
      vector<string>::iterator NewIt=NewCommands.begin();
      while (OldIt!=OldCommands.end())
      {
        if (pOldMakefile->ExpandExpression(*OldIt)!=pNewMakefile->ExpandExpression(*NewIt))
        {
          string ErrorMessage = string("Implicit Rule '") + DepName + "' defined twice with different commands\n";
          ErrorMessage += "Command 1: makedir = " + pOldMakefile->GetMakeDir()->GetQuotedFullFileName()+ "\n";
          ErrorMessage += "  " + pOldMakefile->ExpandExpression(*OldIt) + "\n";
          ErrorMessage += "Command 2: makedir = " + pNewMakefile->GetMakeDir()->GetQuotedFullFileName()+ "\n";
          ErrorMessage += "  " + pNewMakefile->ExpandExpression(*NewIt);
          throw ErrorMessage;
        }
        OldIt++;
        NewIt++;
      }
      #endif
      return true;
    }
    SecIt++;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
void IMPLICITRULE::AddImplicitRule(const refptr<fileinfo> &Target,const vector< refptr<fileinfo> > &Deps,refptr<rule> Rule)
{
  vector<pair<string,refptr<rule> > >& ImplicitRule=m_ImplicitRules[Target->GetFullFileName()];
  if (Deps.size())
  {
    vector< refptr<fileinfo> >::const_iterator DepIt=Deps.begin();
    while (DepIt!=Deps.end())
    {
      const string &DepName=(*DepIt)->GetFullFileName();
      if (!FindDep(DepName,ImplicitRule,Rule))
        ImplicitRule.push_back(pair<string,refptr<rule> >(DepName,Rule));
      DepIt++;
    }
  }
  else
  {
    if (!FindDep(g_EmptyString,ImplicitRule,Rule))
      ImplicitRule.push_back(pair<string,refptr<rule> >(g_EmptyString,Rule));
  }
}

///////////////////////////////////////////////////////////////////////////////
void IMPLICITRULE::SearchImplicitRule(const refptr<fileinfo> &Target,vector< pair<refptr<fileinfo>,refptr<rule> > >&Result)
{
  string TargetFileName=Target->GetFullFileName();

  map< string, vector<pair<string,refptr<rule> > > >::iterator ImpRegExIt=m_ImplicitRules.begin();
  while (ImpRegExIt!=m_ImplicitRules.end())
  {
    matchres Res;

    if (PercentMatch(TargetFileName,ImpRegExIt->first,&Res))
    {
      vector<pair<string,refptr<rule> > >::iterator ResIt=ImpRegExIt->second.begin();
      while (ResIt!=ImpRegExIt->second.end())
      {
#ifdef _DEBUG
        if (!ResIt->second)
        {
          throw string("No rhs for implicit rule : ") + ImpRegExIt->first;
        }
#endif
        ResIt->second->SetStem(Res.m_Stem);
        if (!ResIt->first.empty())
        {
          string Dependent=ReplaceWithStem(ResIt->first,Res.m_Stem);
          Result.push_back(pair<refptr<fileinfo>,refptr<rule> >(GetFileInfo(Dependent),ResIt->second));
        }
        else
          Result.push_back(pair<refptr<fileinfo>,refptr<rule> >(NullFileInfo,ResIt->second));
        ResIt++;
      }
    }
    ImpRegExIt++;
  }
  return;
}

///////////////////////////////////////////////////////////////////////////////
bool rule::operator != (const rule &Other)
{
  if (m_Commands.size()!=Other.m_Commands.size())
    return true;

  vector<string>::const_iterator It=m_Commands.begin();
  vector<string>::const_iterator OtherIt=Other.m_Commands.begin();
  while (It!=m_Commands.end())
  {
    if (m_pMakefile->ExpandExpression(*It)!=Other.m_pMakefile->ExpandExpression(*OtherIt))
      return true;
    It++;
    OtherIt++;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
void rule::SetTargetsIsBuild(uint32 Md5_32)
{
  vector< fileinfo* >::iterator It=m_Targets.begin();
  while (It!=m_Targets.end())
  {
    (*It)->SetCommandsMd5_32(Md5_32);
    (*It)->SetBuild();
    m_pMakefile->AddTarget(*It);
    It++;
  }
}


#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
void IMPLICITRULE::PrintImplicitRules()
{
  map< string, vector<pair<string,refptr<rule> > > >::iterator ImpRegExIt=m_ImplicitRules.begin();
  while (ImpRegExIt!=m_ImplicitRules.end())
  {
    vector<pair<string,refptr<rule> > >::iterator SecIt=ImpRegExIt->second.begin();
    cout << ImpRegExIt->first << " :\n";
    while (SecIt!=ImpRegExIt->second.end())
    {
      cout << "  : " << SecIt->first <<endl;
      if (SecIt->second)
      {
        SecIt->second->PrintCommands();
      }
      else
      {
        cout << "  No rhs\n";
      }
      SecIt++;
    }
    ImpRegExIt++;
  }
  return;
}

///////////////////////////////////////////////////////////////////////////////
void rule::PrintCommands(refptr<fileinfo> Target) const
{
  if (Target)
    m_pMakefile->SetRuleThatIsBuild(Target);

  vector<string>::const_iterator pCommandIt=m_Commands.begin();
  while (pCommandIt!=m_Commands.end())
  {
    cout<<g_SpaceString<<*pCommandIt<<endl;
    if (Target)
    {
      cout<<" ("<<m_pMakefile->ExpandExpression(*pCommandIt)<<")\n";
    }
    pCommandIt++;
  }
}

#endif


