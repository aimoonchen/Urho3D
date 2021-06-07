//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

package io.urho3d.launcher

import androidx.appcompat.app.AppCompatActivity
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.ExpandableListAdapter
import android.widget.ExpandableListView
import android.widget.SimpleExpandableListAdapter
import io.urho3d.UrhoActivity

class LauncherActivity : AppCompatActivity() {
    private var expandableListView: ExpandableListView? = null
    private var expandableListAdapter: ExpandableListAdapter? = null
    // Filter to only include filename that has an extension
    private fun getScriptNames(path: String) = assets.list(path)!!.filter { it.contains('.') }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_launcher)
        expandableListView = findViewById(R.id.expendableList)
        // Only the sample library is selectable, excluding Urho3DPlayer which is handled separately
        val regex = Regex("^(?:Urho3D.*|.+_shared)\$")
        val libraryNames = UrhoActivity.getLibraryNames(this)
        val items = mutableMapOf("C++" to libraryNames.filterNot { regex.matches(it) }.sorted())
        if (libraryNames.find { it == "Urho3DPlayer" } != null) {
            items.putAll(
                mapOf(
                    // FIXME: Should not assume both scripting subsystems are enabled in the build
                    "AngelScript" to getScriptNames("Data/Scripts"),
                    "Lua" to getScriptNames("Data/LuaScripts")
                )
            )
        }
        items.filterValues { it.isEmpty() }.forEach { items.remove(it.key) }

        expandableListAdapter = SimpleExpandableListAdapter(
                this,
                items.map {
                    mapOf("api" to it.key, "info" to "Click to expand/collapse")
                },
                android.R.layout.simple_expandable_list_item_2,
                arrayOf("api", "info"),
                intArrayOf(android.R.id.text1, android.R.id.text2),
                items.map {
                    it.value.map { name ->
                        mapOf("item" to name)
                    }
                },
                R.layout.launcher_list_item,
                arrayOf("item"),
                intArrayOf(android.R.id.text1)
            )
        expandableListView!!.setAdapter(expandableListAdapter)

        expandableListView!!.setOnChildClickListener{ parent, _, groupPos, childPos, _ ->
            launch((parent.getExpandableListAdapter().getChild(groupPos, childPos) as Map<String, String>)["item"])
            false
        }

        // Pass the argument to the main activity, if any
        launch(intent.getStringExtra(MainActivity.argument))
    }

    private fun launch(argument: String?) {
        if (argument != null) {
            startActivity(
                Intent(this, MainActivity::class.java)
                    .putExtra(
                        MainActivity.argument,
                        if (argument.contains('.')) {
                            if (argument.endsWith(".as")) "Urho3DPlayer:Scripts/$argument"
                            else "Urho3DPlayer:LuaScripts/$argument"
                        } else argument
                    )
            )
        }
    }

}
