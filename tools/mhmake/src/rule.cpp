/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2010 marha@sourceforge.net
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

vector<pair<fileinfo *, vector<pair< fileinfo *,refptr<rule> > > > > IMPLICITRULE::m_ImplicitRules;

makecommand g_MakeCommand;  // Order is important since sm_Statics is using g_MakeCommand
const string g_QuoteString("\""); // Order is important since sm_Statics is using g_QuoteString
loadedmakefile::loadedmakefile_statics loadedmakefile::sm_Statics;

///////////////////////////////////////////////////////////////////////////////
static bool FindDep(fileinfo *pDep, vector<pair<fileinfo*,refptr<rule> > >*pImplicitRule,refptr<rule> &Rule)
{
  vector<pair<fileinfo*,refptr<rule> > >::iterator SecIt=pImplicitRule->begin();
  while (SecIt!=pImplicitRule->end())
  {
    if (SecIt->first==pDep)
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
          ErrorMessage += "Implicit Rule '"+ pDep->GetFullFileName() + "' defined twice with different commands\n";
        else
          ErrorMessage += "Implicit Rule '"+ pDep->GetFullFileName() + "' defined twice with same commands\n";
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
          string ErrorMessage = string("Implicit Rule '") + pDep->GetFullFileName() + "' defined twice with different commands\n";
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
void IMPLICITRULE::AddImplicitRule(fileinfo *pTarget,const vector<fileinfo*> &Deps,refptr<rule> Rule)
{
  // first search if there is already the same target in the current list of implicit rules
  vector<pair<fileinfo*,refptr<rule> > >* pImplicitRule=NULL;
  vector<pair<fileinfo *, vector<pair< fileinfo *,refptr<rule> > > > >::iterator RuleIt=m_ImplicitRules.begin();
  while (RuleIt!=m_ImplicitRules.end())
  {
    if (pTarget==RuleIt->first)
    {
      pImplicitRule=&(RuleIt->second);
    }
    RuleIt++;
  }
  if (!pImplicitRule)
{
    // Add a new entry
    m_ImplicitRules.push_back(pair<fileinfo *, vector<pair< fileinfo *,refptr<rule> > > >(pTarget,vector<pair<fileinfo*,refptr<rule> > >()));
    pImplicitRule=&((m_ImplicitRules.end()-1)->second);
  }

  if (Deps.size())
  {
    vector<fileinfo*>::const_iterator DepIt=Deps.begin();
    while (DepIt!=Deps.end())
    {
      #ifdef _DEBUG
      if (*DepIt==pTarget)
        throw(string("Implicit rule : ")+pTarget->GetFullFileName()+" is directly dependent on itself. This is not allowed.");
      #endif
      if (!FindDep(*DepIt,pImplicitRule,Rule))
        pImplicitRule->push_back(pair<fileinfo*,refptr<rule> >(*DepIt,Rule));
      DepIt++;
    }
  }
  else
  {
    if (!FindDep(NULL,pImplicitRule,Rule))
      pImplicitRule->push_back(pair<fileinfo*,refptr<rule> >((fileinfo*)NULL,Rule));
  }
}

///////////////////////////////////////////////////////////////////////////////
void IMPLICITRULE::SearchImplicitRule(const fileinfo *pTarget,vector< pair<fileinfo*,refptr<rule> > >&Result)
{
  string TargetFileName=pTarget->GetFullFileName();

  vector<pair<fileinfo *, vector<pair< fileinfo *,refptr<rule> > > > >::iterator ImpRegExIt=m_ImplicitRules.begin();
  while (ImpRegExIt!=m_ImplicitRules.end())
  {
    matchres Res;

    if (PercentMatch(TargetFileName,ImpRegExIt->first->GetFullFileName(),&Res))
    {
      vector<pair<fileinfo*,refptr<rule> > >::iterator ResIt=ImpRegExIt->second.begin();
      while (ResIt!=ImpRegExIt->second.end())
      {
#ifdef _DEBUG
        if (!ResIt->second)
        {
          throw string("No rhs for implicit rule : ") + ImpRegExIt->first->GetFullFileName();
        }
#endif
        ResIt->second->SetStem(Res.m_Stem);
        if (ResIt->first!=NULL)
        {
          string Dependent=ReplaceWithStem(ResIt->first->GetFullFileName(),Res.m_Stem);
          Result.push_back(pair<fileinfo*,refptr<rule> >(GetFileInfo(Dependent,ResIt->second->GetMakefile()->GetMakeDir()),ResIt->second));
        }
        else
          Result.push_back(pair<fileinfo*,refptr<rule> >((fileinfo*)NULL,ResIt->second));
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
///////////////////////////////////////////////////////////////////////////////
void rule::SetTargetsIsBuilding(const fileinfo *pSrc)
{
  vector< fileinfo* >::iterator It=m_Targets.begin();
  while (It!=m_Targets.end())
  {
    if ((*It)!=pSrc)
      (*It)->SetBuilding(false);
    It++;
  }
}


#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
void IMPLICITRULE::PrintImplicitRules()
{
  vector<pair<fileinfo *, vector<pair< fileinfo *,refptr<rule> > > > >::iterator ImpRegExIt=m_ImplicitRules.begin();
  while (ImpRegExIt!=m_ImplicitRules.end())
  {
    vector<pair<fileinfo*,refptr<rule> > >::iterator SecIt=ImpRegExIt->second.begin();
    cout << ImpRegExIt->first->GetFullFileName() << " :\n";
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
void rule::PrintCommands(fileinfo *pTarget) const
{
  if (pTarget)
    m_pMakefile->SetRuleThatIsBuild(pTarget);

  vector<string>::const_iterator pCommandIt=m_Commands.begin();
  while (pCommandIt!=m_Commands.end())
  {
    cout<<g_SpaceString<<*pCommandIt<<endl;
    if (pTarget)
    {
      cout<<" ("<<m_pMakefile->ExpandExpression(*pCommandIt)<<")\n";
    }
    pCommandIt++;
  }
}

#endif


