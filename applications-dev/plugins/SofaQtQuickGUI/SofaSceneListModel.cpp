/*
Copyright 2015, Anatoscope

This file is part of sofaqtquick.

sofaqtquick is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sofaqtquick is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sofaqtquick. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SofaSceneListModel.h"

#include <QStack>
#include <iostream>

namespace sofa
{

namespace qtquick
{

using namespace sofa::defaulttype;
using namespace sofa::core::objectmodel;
using namespace sofa::simulation;

SofaSceneListModel::SofaSceneListModel(QObject* parent) : QAbstractListModel(parent), MutationListener(),
    myItems(),
    myUpdatedCount(0),
    myIsDirty(true),
    mySofaScene(nullptr)
{

}

SofaSceneListModel::~SofaSceneListModel()
{
    clear();
}

void SofaSceneListModel::update()
{
    if(!myIsDirty)
        return;

    int count = rowCount();

    dataChanged(createIndex(0, 0), createIndex(qMin(count, myUpdatedCount) - 1, 0));

    int changeNum = count - myUpdatedCount;
    if(changeNum > 0)
    {
        beginInsertRows(QModelIndex(), myUpdatedCount, count - 1);
        endInsertRows();
    }
    else if(changeNum < 0)
    {
        beginRemoveRows(QModelIndex(), count, myUpdatedCount - 1);
        endRemoveRows();
    }

    myUpdatedCount = count;

    myIsDirty = false;
}

void SofaSceneListModel::handleSceneChange(SofaScene*)
{
    clear();
    if(mySofaScene)
    {
        if(mySofaScene->isReady())
        {
            addChild(0, mySofaScene->sofaSimulation()->GetRoot().get());
            update();
        }

        connect(mySofaScene, &SofaScene::statusChanged, this, [this]() {
            clear();

            if(SofaScene::Ready == mySofaScene->status())
                addChild(0, mySofaScene->sofaSimulation()->GetRoot().get());

            update();
        });
    }
}

void SofaSceneListModel::clear()
{
    if(myItems.isEmpty())
        return;

    for(Item& item : myItems)
        if(item.base == item.node)
        {
            Node* node = static_cast<Node*>(item.node);
            if(node)
                node->removeListener(this);
        }

    myItems.clear();

    markDirty();
    update();
}

SofaSceneListModel::Item SofaSceneListModel::buildNodeItem(SofaSceneListModel::Item* parent, BaseNode* node) const
{
    SofaSceneListModel::Item item;

    item.parent = parent;
    item.multiparent = node->getNbParents() > 1;
    item.depth = parent ? parent->depth + 1 : 0;
    item.visibility = !parent ? Visible : (((Hidden | Collapsed) & parent->visibility) ? Hidden : Visible);
    item.base = node;
    item.object = 0;
    item.node = node;

    return item;
}

SofaSceneListModel::Item SofaSceneListModel::buildObjectItem(SofaSceneListModel::Item* parent, BaseObject* object) const
{
    SofaSceneListModel::Item item;

    item.parent = parent;
    item.multiparent = false;
    item.depth = parent ? parent->depth + 1 : 0;
    item.visibility = !parent ? Visible : (((Hidden | Collapsed) & parent->visibility) ? Hidden : Visible);
    item.base = object;
    item.object = object;
    item.node = parent->node;

    return item;
}

void SofaSceneListModel::setSofaScene(SofaScene* newSofaScene)
{
    if(newSofaScene == mySofaScene)
        return;

    if(mySofaScene)
        disconnect(mySofaScene);

    mySofaScene = newSofaScene;

    handleSceneChange(mySofaScene);

    sofaSceneChanged(newSofaScene);
}

int SofaSceneListModel::computeModelRow(int itemRow) const
{
    if(itemRow >= 0 && itemRow < myItems.size())
    {
        int currentModelRow = -1;

        int currentItemRow = 0;
        for(const Item& item : myItems)
        {
            if(!(item.visibility & Visibility::Hidden))
                ++currentModelRow;

            if(currentItemRow == itemRow)
                return currentModelRow;

            ++currentItemRow;
        }
    }

    return -1;
}

int SofaSceneListModel::computeItemRow(int modelRow) const
{
    if(modelRow >= 0 && modelRow < rowCount())
    {
        int currentModelRow = -1;

        int currentItemRow = 0;
        for(const Item& item : myItems)
        {
            if(!(item.visibility & Visibility::Hidden))
                ++currentModelRow;

            if(currentModelRow == modelRow)
                return currentItemRow;

            ++currentItemRow;
        }
    }

    return -1;
}

int SofaSceneListModel::computeItemRow(const QModelIndex& index) const
{
    if(index.isValid())
        return computeItemRow(index.row());

    return -1;
}

int SofaSceneListModel::rowCount(const QModelIndex & /*parent*/) const
{
    int count = 0;
    for(const Item& item : myItems)
    {
        if(!(item.visibility & Visibility::Hidden))
            ++count;
    }

    return count;
}

QVariant SofaSceneListModel::data(const QModelIndex& index, int role) const
{
    if(!mySofaScene || !mySofaScene->isReady())
        return QVariant("");

    int row = computeItemRow(index);

    if(-1 == row)
    {
        msg_error("SofaQtQuickGUI") << "Invalid index";
        return QVariant("");
    }

    const Item& item = myItems[row];
    bool multiparent = item.multiparent;
    int depth = item.depth;
    int visibility = item.visibility;
    Base* base = item.base;
    BaseObject* object = item.object;
    BaseContext* baseContext = item.node->getContext();

    if(0 == base)
    {
        msg_error("SofaQtQuickGUI") << "Item is empty";
        return QVariant("");
    }

    switch(role)
    {
    case NameRole:
        return QVariant::fromValue(QString::fromStdString(base->getName()));
    case MultiParentRole:
        return QVariant::fromValue(multiparent);
    case DepthRole:
        return QVariant::fromValue(depth);
    case VisibilityRole:
        return Visible == visibility && !baseContext->isActive() ? QVariant::fromValue((int) Collapsed | Disabled) : QVariant::fromValue(visibility);
    case TypeRole:
        return QVariant::fromValue(QString::fromStdString(base->getClass()->className));
    case IsNodeRole:
        return QVariant::fromValue(0 == object);
    case CollapsibleRole:
        return QVariant::fromValue(0 == object && item.children.size() != 0);
    default:
        msg_error("SofaQtQuickGUI") << "Role unknown";
    }

    return QVariant("");
}

QHash<int,QByteArray> SofaSceneListModel::roleNames() const
{
    QHash<int,QByteArray> roles;

    roles[NameRole]         = "name";
    roles[MultiParentRole]  = "multiparent";
    roles[DepthRole]        = "depth";
    roles[VisibilityRole]   = "visibility";
    roles[TypeRole]         = "type";
    roles[IsNodeRole]       = "isNode";
    roles[CollapsibleRole]  = "collapsible";

    return roles;
}

QVariant SofaSceneListModel::get(int row) const
{
    QVariantMap object;

    if(row < 0 || row > myItems.size())
    {
        msg_error("SofaQtQuickGUI") << "Invalid index";
        return object;
    }

    QHash<int,QByteArray> roles = roleNames();
    for(auto roleIt = roles.begin(); roleIt != roles.end(); ++roleIt)
        object.insert(roleIt.value(), data(createIndex(row, 0), roleIt.key()));

    return object;
}

void SofaSceneListModel::setCollapsed(int row, bool collapsed)
{
    row = computeItemRow(row);
    if(-1 == row)
    {
        msg_error("SofaQtQuickGUI") << "Invalid index";
        return;
    }

    Item& item = myItems[row];

    BaseContext* baseContext = item.node->getContext();
    if(!baseContext->isActive())
    {
        collapsed = true;
    }
    else
    {
        item.visibility = collapsed;
    }

    QStack<Item*> children;
    for(int i = 0; i < item.children.size(); ++i)
        children.append(item.children[i]);

    while(!children.isEmpty())
    {
        Item* child = children.pop();
        if(collapsed)
            child->visibility |= Hidden;
        else
            child->visibility ^= Hidden;

        if(!(Collapsed & child->visibility))
            for(int i = 0; i < child->children.size(); ++i)
                children.append(child->children[i]);
    }

    markDirty();
    update();
}

SofaComponent* SofaSceneListModel::getComponentById(int modelRow) const
{
    int itemRow = computeItemRow(modelRow);
    if(-1 == itemRow)
    {
        msg_error("SofaQtQuickGUI") << "Invalid index";
        return nullptr;
    }

    return new SofaComponent(mySofaScene, myItems.at(itemRow).base);
}

int SofaSceneListModel::getComponentId(SofaComponent* sofaComponent) const
{
    Base* base = nullptr;
    if(sofaComponent)
        base = sofaComponent->base();

    if(base)
        for(int i = 0; i < myItems.size(); ++i)
            if(base == myItems[i].base)
                return computeModelRow(i);

    return -1;
}

QList<int> SofaSceneListModel::computeFilteredRows(const QString& filter) const
{
    QList<int> filteredRows;

    if(!filter.isEmpty())
        for(int i = 0; i < myItems.size(); ++i)
        {
            const Item& item = myItems[i];

            QString name = QString::fromStdString(item.base->getName());
            if(-1 != name.indexOf(filter, 0, Qt::CaseInsensitive))
                filteredRows.append(computeItemRow(i));
        }

    return filteredRows;
}

bool SofaSceneListModel::isAncestor(SofaSceneListModel::Item* ancestor, SofaSceneListModel::Item* child)
{
    if(!child)
        return false;

    if(!ancestor)
        return true;

    Item* parent = child;
    while((parent = parent->parent))
        if(ancestor == parent)
            return true;

    return false;
}

void SofaSceneListModel::addChild(Node* parent, Node* child)
{
    if(!child)
        return;

    //std::cerr << "++" << child->getName().c_str() << "(" << child << ")" << " ; " << (parent ? parent->getName().c_str() : "") << "(" << parent << ")" << " > " << std::flush;

    if(!parent)
    {
        myItems.append(buildNodeItem(0, child));
    }
    else
    {
        bool alreadyAdded = false;

        QList<Item>::iterator parentItemIt = myItems.begin();
        while(parentItemIt != myItems.end())
        {
            if(parent == parentItemIt->base)
            {
                Item* parentItem = &*parentItemIt;

                QList<Item>::iterator itemIt = parentItemIt;
                while(++itemIt != myItems.end())
                {
                    Item* childItem = &*itemIt;
                    if(childItem->base == child)
                    {
                        alreadyAdded = true;
                        break;
                    }

                    if(!isAncestor(parentItem, childItem))
                        break;
                }

                if(alreadyAdded)
                    break;

                QList<Item>::iterator childItemIt = myItems.insert(itemIt, buildNodeItem(parentItem, child));
                parentItem->children.append(&*childItemIt);

                parentItemIt = childItemIt;
                break;
            }
            else
                ++parentItemIt;
        }
    }

    MutationListener::addChild(parent, child);

    markDirty();
}

void SofaSceneListModel::removeChild(Node* parent, Node* child)
{
    if(!child)
        return;

    MutationListener::removeChild(parent, child);

    //std::cerr << "--" << child->getName().c_str() << "(" << child << ")" << " ; " << (parent ? parent->getName().c_str() : "") << "(" << parent << ")" << " > " << std::flush;

    QList<Item>::iterator itemIt = myItems.begin();
    while(itemIt != myItems.end())
    {
        if(child == itemIt->base)
        {
            Item* parentItem = itemIt->parent;
            if((!parent && !parentItem) || (parent && parentItem && parent == parentItem->base))
            {
                //std::cerr << &*itemIt << std::endl;

                if(parentItem)
                {
                    int index = parentItem->children.indexOf(&*itemIt);
                    if(-1 != index)
                        parentItem->children.remove(index);
                }

                itemIt = myItems.erase(itemIt);
                break;
            }
            else
                ++itemIt;
        }
        else
        {
            ++itemIt;
        }
    }

    markDirty();
}

//void SceneListModel::moveChild(Node* previous, Node* parent, Node* child)
//{

//}

void SofaSceneListModel::addObject(Node* parent, BaseObject* object)
{
    if(!object || !parent)
        return;

    //std::cerr << "+" << object->getName().c_str() << "(" << object << ")" << " ; " << (parent ? parent->getName().c_str() : "") << "(" << parent << ")" << " > " << std::flush;

    bool alreadyAdded = false;
    for(auto it = myItems.begin(); it != myItems.end(); ++it)
        if(it->base == object)
        {
            alreadyAdded = true;
            break;
        }

    if(!alreadyAdded)
    {
        QList<Item>::iterator parentItemIt = myItems.begin();
        while(parentItemIt != myItems.end())
        {
            if(parent == parentItemIt->base)
            {
                Item* parentItem = &*parentItemIt;

                QList<Item>::iterator itemIt = parentItemIt;
                while(++itemIt != myItems.end())
                    if(parentItem != itemIt->parent || !itemIt->object)
                        break;

                QList<Item>::iterator childItemIt = myItems.insert(itemIt, buildObjectItem(parentItem, object));
                parentItem->children.append(&*childItemIt);
                parentItemIt = childItemIt;

                break;
            }
            else
                ++parentItemIt;
        }

        markDirty();
    }

    MutationListener::addObject(parent, object);
}

void SofaSceneListModel::removeObject(Node* parent, BaseObject* object)
{
    if(!object || !parent)
        return;

    MutationListener::removeObject(parent, object);

    //std::cerr << "-" << object->getName().c_str() << "(" << object << ")" << " ; " << (parent ? parent->getName().c_str() : "") << "(" << parent << ")" << " > " << std::flush;

    QList<Item>::iterator itemIt = myItems.begin();
    while(itemIt != myItems.end())
    {
        if(object == itemIt->base)
        {
            Item* parentItem = itemIt->parent;
            if(parentItem && parent == parentItem->base)
            {
                //std::cerr << &*itemIt << " - " << &*itemIt->parent << std::endl;
                if(parentItem)
                {
                    int index = parentItem->children.indexOf(&*itemIt);

                    if(-1 != index)
                        parentItem->children.remove(index);
                }

                itemIt = myItems.erase(itemIt);
                break;
            }
            else
                ++itemIt;
        }
        else
        {
            ++itemIt;
        }
    }

	markDirty();
}

//void SceneListModel::moveObject(Node* previous, Node* parent, BaseObject* object)
//{
//	markDirty();
//}

void SofaSceneListModel::addSlave(BaseObject* master, BaseObject* slave)
{
    MutationListener::addSlave(master, slave);

	markDirty();
}

void SofaSceneListModel::removeSlave(BaseObject* master, BaseObject* slave)
{
    MutationListener::removeSlave(master, slave);

	markDirty();
}

}

}
