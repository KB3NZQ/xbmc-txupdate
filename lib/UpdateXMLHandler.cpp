/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "UpdateXMLHandler.h"
#include "Log.h"
#include "HTTPUtils.h"
#include "JSONHandler.h"

using namespace std;

CUpdateXMLHandler::CUpdateXMLHandler()
{};

CUpdateXMLHandler::~CUpdateXMLHandler()
{};

bool CUpdateXMLHandler::LoadXMLToMem (std::string rootDir)
{
  std::string UpdateXMLFilename = rootDir  + DirSepChar + "xbmc-txupdate.xml";
  TiXmlDocument xmlUpdateXML;

  CHTTPHandler HTTPHandler;
  std::string strtemp;
//  strtemp = HTTPHandler.GetURLToSTR("https://raw.github.com/xbmc/xbmc/master/language/English/strings.po");
//  strtemp = HTTPHandler.GetURLToSTR("https://www.transifex.com/api/2/project/update-test/resource/visualization-projectm/translation/hu/?file", "un", "pw");
  strtemp = HTTPHandler.GetURLToSTR("https://www.transifex.com/api/2/project/XBMC-Main-Frodo/resources/", "alanwww1", "pdance10");
  printf("%s, strlength: %i", strtemp.c_str(), strtemp.size());

//  char cstrtemp[300];
//  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  JSONHandler.ParseResources(strtemp);

//  HTTPHandler.Cleanup();
//  HTTPHandler.ReInit();
//  HTTPHandler.GetURLToFILE(rootDir + "test2.po", "https://raw.github.com/xbmc/xbmc/master/language/English/strings.po");
//  HTTPHandler.GetURLToFILE(rootDir + "test1.po","https://www.transifex.com/api/2/project/update-test/resource/visualization-projectm/translation/hu/?file", "login", "passw");

  if (!xmlUpdateXML.LoadFile(UpdateXMLFilename.c_str()))
  {
    CLog::Log(logINFO, "UpdXMLHandler: No update.xml file exists, we will create one later");
    return false;
  }

  CLog::Log(logINFO, "UpdXMLHandler: Succesfuly found the update.xml file");

  CXMLResdata currResData;

  TiXmlElement* pRootElement = xmlUpdateXML.RootElement();

  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="resources")
  {
    CLog::Log(logINFO, "UpdXMLHandler: No root element called \"resources\" in xml file. We will create it");
    return false;
  }

  const TiXmlElement *pChildResElement = pRootElement->FirstChildElement("resource");
  while (pChildResElement && pChildResElement->FirstChild())
  {
    std::string strResName = pChildResElement->Attribute("name");
    if (pChildResElement->FirstChild())
    {
      const TiXmlElement *pChildURLElement = pChildResElement->FirstChildElement("upstreamURL");
      if (pChildURLElement && pChildURLElement->FirstChild())
        currResData.strUptreamURL = pChildURLElement->FirstChild()->Value();
      const TiXmlElement *pChildUpstrLElement = pChildResElement->FirstChildElement("upstreamLangs");
      if (pChildUpstrLElement && pChildUpstrLElement->FirstChild())
        currResData.strLangsFromUpstream = pChildUpstrLElement->FirstChild()->Value();
      m_mapXMLResdata[strResName] = currResData;
      CLog::Log(logINFO, "UpdXMLHandler: found resource in update.xml file: %s", strResName.c_str());
    }
    pChildResElement = pChildResElement->NextSiblingElement("resource");
  }

  return true;
};

int CUpdateXMLHandler::GetResType(std::string ResRootDir)
{
  if ((FileExist(ResRootDir + "addon.xml")) && (FileExist(ResRootDir + "resources" + DirSepChar + "language" +
      DirSepChar + "English" + DirSepChar + "strings.po")))
    return ADDON;
  else if ((FileExist(ResRootDir + "addon.xml")) && (FileExist(ResRootDir + "language" + DirSepChar + "English" +
           DirSepChar + "strings.po")))
    return SKIN;
  else if (FileExist(ResRootDir + "addon.xml"))
    return ADDON_NOSTRINGS;
  else if (FileExist(ResRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h"))
    return CORE;
  else
    return UNKNOWN;
};

void CUpdateXMLHandler::GetResourcesFromDir(std::string strProjRootDir)
{
  CXMLResdata emptyResData;
  DIR* Dir;
  struct dirent *DirEntry;
  Dir = opendir(strProjRootDir.c_str());

  while((DirEntry=readdir(Dir)))
  {
    if (DirEntry->d_type != DT_DIR || DirEntry->d_name[0] == '.')
      continue;
    if (m_mapXMLResdata.find(DirEntry->d_name) == m_mapXMLResdata.end())
    {
      CLog::Log(logINFO, "UpdXMLHandler: New resource detected: %s", DirEntry->d_name);
      m_mapXMLResdata[DirEntry->d_name] = emptyResData;
    }
  }
};

void CUpdateXMLHandler::SaveMemToXML(std::string rootDir)
{
  std::string UpdateXMLFilename = rootDir  + DirSepChar + "xbmc-txupdate.xml";
  TiXmlDocument doc;
  TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
  TiXmlElement * pRootElement = new TiXmlElement( "resources" );

  for (itXMLResdata = m_mapXMLResdata.begin(); itXMLResdata != m_mapXMLResdata.end(); itXMLResdata++)
  {
    TiXmlElement * pChildResElement = new TiXmlElement( "resource" );
    pChildResElement->SetAttribute("name", itXMLResdata->first.c_str());

    TiXmlElement * pChildURLElement = new TiXmlElement( "upstreamURL" );
    TiXmlText * textURL = new TiXmlText( itXMLResdata->second.strUptreamURL.c_str() );
    pChildURLElement->LinkEndChild(textURL);
    pChildResElement->LinkEndChild(pChildURLElement);

    TiXmlElement * pChildUpstrLElement = new TiXmlElement( "upstreamLangs" );
    TiXmlText * textUpsLang = new TiXmlText( itXMLResdata->second.strLangsFromUpstream.c_str());
    pChildUpstrLElement->LinkEndChild(textUpsLang);
    pChildResElement->LinkEndChild(pChildUpstrLElement);

    pRootElement->LinkEndChild(pChildResElement);
  }

  doc.LinkEndChild( decl );
  doc.LinkEndChild( pRootElement );
  doc.SaveFile(UpdateXMLFilename.c_str());
};