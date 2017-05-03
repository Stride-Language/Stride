/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "stridesystem.hpp"

#include "propertynode.h"
#include "declarationnode.h"
#include "pythonproject.h"


StrideSystem::StrideSystem(QString strideRoot, QString systemName,
                           int majorVersion, int minorVersion,
                           QMap<QString, QString> importList) :
    m_strideRoot(strideRoot), m_systemName(systemName), m_majorVersion(majorVersion), m_minorVersion(minorVersion)
{
    QString versionString = QString("%1.%2").arg(m_majorVersion).arg(m_minorVersion);
    QString fullPath = QDir(strideRoot + QDir::separator()
                            + "systems" + QDir::separator()
                            + systemName + QDir::separator()
                            + versionString).absolutePath();
    QString systemFile = fullPath + QDir::separator() + "System.stride";

    m_library.setLibraryPath(strideRoot, importList);
    if (QFile::exists(systemFile)) {
        AST *systemTree = AST::parseFile(systemFile.toStdString().c_str());
        if (systemTree) {
            parseSystemTree(systemTree);

            // Add subpaths for included modules
            vector<string> subPaths;
            subPaths.push_back("");
            QMapIterator<QString, QString> it(importList);
            while (it.hasNext()) {
                it.next();
                subPaths.push_back(it.key().toStdString());
            }
            // Iterate through platforms reading them. Should optimize this to
            // not reread platform if already done.
            // Only required if includes have changed
            for(StridePlatform *platform: m_platforms) {
                QStringList nameFilters;
                nameFilters.push_back("*.stride");
                string platformPath = platform->buildLibPath(m_strideRoot.toStdString());
                foreach(string subPath, subPaths) {
                    QString includeSubPath = QString::fromStdString(platformPath + "/" + subPath);
                    QStringList libraryFiles =  QDir(includeSubPath).entryList(nameFilters);
                    foreach (QString file, libraryFiles) {
                        QString fileName = includeSubPath + QDir::separator() + file;
                        AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
                        if(tree) {
                            platform->addTree(file.toStdString(),tree);
                        } else {
                            vector<LangError> errors = AST::getParseErrors();
                            foreach(LangError error, errors) {
                                qDebug() << QString::fromStdString(error.getErrorText());
                            }
                            continue;
                        }
                        QString namespaceName = importList[QString::fromStdString(subPath)];
                        if (!namespaceName.isEmpty()) {
                            foreach(AST *node, tree->getChildren()) {
                                // Do we need to set namespace recursively or would this do?
                                //                            node->setNamespace(namespaceName.toStdString());
                            }
                        }
                    }
                }
//                m_platformPath = fullPath;
//                m_api = PythonTools;
//                m_types = getPlatformTypeNames();
            }
            systemTree->deleteChildren();
            delete systemTree;
        } else {
            qDebug() << "Error parsing system tree in:" << systemFile;
        }
    } else {
        qDebug() << "System file not found:" << systemFile;
    }

//    if (m_api == NullPlatform) {
//        qDebug() << "Platform not found!";
//    }
}

StrideSystem::~StrideSystem()
{
    foreach(auto platform, m_platforms) {
        delete platform;
    }
}

//ListNode *StreamPlatform::getPortsForFunction(QString typeName)
//{
//    foreach(AST* group, m_platform) {
//        foreach(AST *node, group->getChildren()){
//            if (node->getNodeType() == AST::Declaration) {
//                DeclarationNode *block = static_cast<DeclarationNode *>(node);
//                if (block->getObjectType() == "platformModule") {
//                    if (block->getObjectType() == typeName.toStdString()) {
//                        vector<PropertyNode *> ports = block->getProperties();
//                        foreach(PropertyNode *port, ports) {
//                            if (port->getName() == "ports") {
//                                ListNode *portList = static_cast<ListNode *>(port->getValue());
//                                Q_ASSERT(portList->getNodeType() == AST::List);
//                                return portList;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    return NULL;
//}

//DeclarationNode *StreamPlatform::getFunction(QString functionName)
//{
//    QStringList typeNames;
//    foreach(AST* node, m_platform) {
//        if (node->getNodeType() == AST::Declaration) {
//            DeclarationNode *block = static_cast<DeclarationNode *>(block);
//            if (block->getObjectType() == "platformModule"
//                    || block->getObjectType() == "module") {
//                if (block->getName() == functionName.toStdString()) {
//                    return block;
//                }
//            }
//        }
//    }
//    foreach(AST* node, m_library.getNodes()) {

//        if (node->getNodeType() == AST::Declaration) {
//            DeclarationNode *block = static_cast<DeclarationNode *>(node);
//            if (block->getObjectType() == "module") {
//                if (block->getName() == functionName.toStdString()) {
//                    return block;
//                }
//            }
//        }
//    }
//    return NULL;
//}

void StrideSystem::parseSystemTree(AST *systemTree)
{
    vector<string> usedPlatformNames;
    vector<map<string, string>> platformDefinitions;
    vector<string> platformDefinitionNames;
    vector<AST *> connectionDefinitions;
    for(AST *systemNode:systemTree->getChildren()) {
        if(systemNode->getNodeType() == AST::Declaration) {
            DeclarationNode *declaration = static_cast<DeclarationNode *>(systemNode);
            if (declaration->getObjectType() == "platform") {
                map<string, string> definition;
                vector<PropertyNode *> properties = declaration->getProperties();
                for (PropertyNode *prop: properties) {
                    if (prop->getValue()->getNodeType() == AST::String) {
                        definition[prop->getName()] =
                                static_cast<ValueNode *>(prop->getValue())->getStringValue();
                    }
                }
                platformDefinitions.push_back(definition);
                platformDefinitionNames.push_back(declaration->getName());
            } else if (declaration->getObjectType() == "connection") {
                // FIXME add connections
//                connectionDefinitions.push_back(declaration->deepCopy());
            } else if (declaration->getObjectType() == "system") {
                AST *platforms = declaration->getPropertyValue("platforms");
                if (platforms->getNodeType() == AST::List) {
                    ListNode *platformsList = static_cast<ListNode *>(platforms);
                    for(AST *platformName:platformsList->getChildren()) {
                        if (platformName->getNodeType() == AST::Block) {
                            usedPlatformNames.push_back(static_cast<BlockNode *>(platformName)->getName());
                        }
                    }
                }
            } else {
                qDebug() << "ERROR: Unknown system declaration type";
            }
        }
    }

    // Now connect platforms referenced in system with defined platforms
    for(string usedPlatformName:usedPlatformNames) {
        for (int i = 0; i < platformDefinitionNames.size(); i++) {
            map<string, string> &definition = platformDefinitions.at(i);
            string &name = platformDefinitionNames.at(i);
            if (name == usedPlatformName) {
                string framework, framworkVersion, hardware, hardwareVersion;
                auto defIt = definition.begin();
                while(defIt != definition.end()) {
                    if (defIt->first == "framework") {
                        framework = defIt->second;
                        qDebug() << "Found framework:" << QString::fromStdString(framework);
                    } else if (defIt->first == "frameworkVersion") {
                        framworkVersion = defIt->second;
                    } else if (defIt->first == "hardware") {
                        hardware = defIt->second;
                    } else if (defIt->first == "hardwareVersion") {
                        hardwareVersion = defIt->second;
                    } else {

                    }
                    defIt++;
                }
                StridePlatform *newPlatform = new StridePlatform(framework, framworkVersion, hardware, hardwareVersion);
                m_platforms.push_back(newPlatform);
            } else {
                // TODO add error
            }
        }
    }

}

QStringList StrideSystem::getErrors()
{
    return m_errors;
}

QStringList StrideSystem::getWarnings()
{
    return m_warnings;
}

QStringList StrideSystem::getPlatformTypeNames()
{
    QStringList typeNames;
//    foreach(AST* group, m_platform) {
//        foreach(AST *node, group->getChildren()) {
//            if (node->getNodeType() == AST::Declaration) {
//                DeclarationNode *block = static_cast<DeclarationNode *>(node);
//                if (block->getObjectType() == "platformType") {
//                    ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
//                    if (name) {
//                        Q_ASSERT(name->getNodeType() == AST::String);
//                        typeNames << QString::fromStdString(name->getStringValue());
//                    } else {
//                        qDebug() << "Error. platform Type missing typeName port.";
//                    }
//                }
//            }
//        }
//    }
    vector<AST *> libObjects = m_library.getLibraryMembers();
    foreach(AST *node, libObjects) {
        if (node->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(node);
            if (block->getObjectType() == "platformType"
                    || block->getObjectType() == "type") {
                ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
                Q_ASSERT(name->getNodeType() == AST::String);
                typeNames << QString::fromStdString(name->getStringValue());
            }
        }
    }
    return typeNames;
}

QStringList StrideSystem::getFunctionNames()
{
    QStringList typeNames;
    foreach(AST* node, getBuiltinObjectsReference()) {
        if (node->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(node);
            if (block->getObjectType() == "module") {
                typeNames << QString::fromStdString(block->getName());
            }
        }
    }
    return typeNames;
}

Builder *StrideSystem::createBuilder(QString projectDir)
{
    // FIXME create multiple builders when more than one framework.
    Builder *builder = NULL;
    StridePlatform *platform = m_platforms.at(0); // FIXME support multiple platforms
    if (platform->getAPI() == StridePlatform::PythonTools) {
        QString pythonExec = "python";
        builder = new PythonProject(QString::fromStdString(platform->buildPlatformPath(m_strideRoot.toStdString())),
                                    m_strideRoot, projectDir, pythonExec);
    }/* else if(m_api == StrideSystem::PluginPlatform) {
        QString xmosRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";

        QLibrary pluginLibrary(m_pluginName);
        if (!pluginLibrary.load()) {
            qDebug() << pluginLibrary.errorString();
            return NULL;
        }
        create_object_t create = (create_object_t) pluginLibrary.resolve("create_object");
        if (create) {
            builder = create(m_platformPath, projectDir.toLocal8Bit(), xmosRoot.toLocal8Bit());
        }
        pluginLibrary.unload();
    }*/
    if (builder && !builder->isValid()) {
        delete builder;
        builder = NULL;
    }
    return builder;
}

QString StrideSystem::getPlatformDomain()
{
    QList<AST *> libObjects = getBuiltinObjectsReference();
    foreach(AST *object, libObjects) {
        if (object->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(object);
            if (block->getObjectType() == "constant"
                    && block->getName() == "PlatformDomain") {
                ValueNode *domainValue = static_cast<ValueNode *>(block->getPropertyValue("value"));
                Q_ASSERT(domainValue->getNodeType() == AST::String);
                return QString::fromStdString(domainValue->getStringValue());
            }
        }
    }
    return "";
}

QList<AST *> StrideSystem::getBuiltinObjectsCopy()
{
    QList<AST *> objects;
    QList<AST *> libObjects = getBuiltinObjectsReference();
    foreach(AST *object, libObjects) {
        objects << object->deepCopy();
    }
    return objects;
}

QList<AST *> StrideSystem::getBuiltinObjectsReference()
{
    QList<AST *> objects;
    if (m_platforms.size() > 0) {
        objects = m_platforms.at(0)->getPlatformObjects();
    }
    vector<AST *> libObjects = m_library.getLibraryMembers();
    foreach(AST *object, libObjects) {
        objects << object;
    }
    return objects;
}