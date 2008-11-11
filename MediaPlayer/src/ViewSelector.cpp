/******************************************************************************
 * Copyright (C) 2008 James McKaskill <jmckaskill@gmail.com>                  *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "stdafx.h"

#include <boost/bind.hpp>
#include <qsmp/ViewSelector.h>
#include <qsmp/ViewSelector.moc>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ViewSelector::ViewSelector(QWidget* parent_widget)
: parent_widget_(parent_widget)
{
  model_ = new Model(boost::bind(&ViewSelector::tree,this));
  model_->set_clicked(boost::bind(&ViewSelector::EntryDoubleClicked,this,_1));
  model_->SetView(this);
  setModel(model_);
}

//-----------------------------------------------------------------------------

void ViewSelector::AddViewEntry(boost::function<QLayout* ()> new_view,
                                const std::string&           text)
{
  ViewEntry view;
  view.new_view_ = new_view;
  view.text_     = QString::fromStdString(text);
  tree_.push_back(view);
  model_->reset();
}

//-----------------------------------------------------------------------------

void ViewSelector::EntryDoubleClicked(const ViewSelector::Tree& node)
{
  delete parent_widget_->layout();
  parent_widget_->setLayout(node.get().new_view_());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END