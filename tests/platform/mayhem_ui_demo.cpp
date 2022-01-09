/***************************************************************************       
 *   Copyright (C) 2020 by Federico Amedeo Izzo IU2NUO,                    *       
 *                         Niccolò Izzo IU2KIN                             *       
 *                         Frederik Saraci IU2NRO                          *       
 *                         Silvano Seva IU2KWO                             *       
 *                                                                         *       
 *   This program is free software; you can redistribute it and/or modify  *       
 *   it under the terms of the GNU General Public License as published by  *       
 *   the Free Software Foundation; either version 3 of the License, or     *       
 *   (at your option) any later version.                                   *       
 *                                                                         *       
 *   This program is distributed in the hope that it will be useful,       *       
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *       
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *       
 *   GNU General Public License for more details.                          *       
 *                                                                         *       
 *   You should have received a copy of the GNU General Public License     *       
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *       
 ***************************************************************************/  

#include "mayhem_ui_demo.hpp"
#include <cstring>

namespace ui
{

    UIDemoView::UIDemoView(NavigationView &nav)
    {
        // Define text widget
        Text hello_world_text_widget(
            // int:x (px), int:y (px), int:width (px), int:height (px)
            {10, 10, 100, 24},
            "Hello world!"
        );
        // Register text widget
        add_child(&hello_world_text_widget);
    }

    void NewAppView::update()
    {
    }
}
