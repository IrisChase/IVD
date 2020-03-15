# IVD - Interactive Visual Design Language

IVD is a declarative GUI programming language and framework implementation. The goal is to make it so that you can describe *how* a interface should work and leave the rest to the computer, facilitating highly dynamic GUI programming, whilst maintaining maintainability.

# Why?

The decision to create IVD was not reached lightly. I knew it would be a huge undertaking (Although I underestimated how huge), and spent considerable time trying to talk myself out of doing it.

I have experienced developing user interfaces in the classical way (Qt), the "modern" way (HTML/CSS/Javascript), and played around a bit with QML, which was somewhat inspiring for this project. All of these had their pros and cons, but none of them seemed to get at the heart of the problem.

# A Quick(ish) and Mostly Incomplete Rundown of IVD
## Events Kinda Suck, State System to The Rescue
One great problem I think is the low level nature of typical GUI events. Take for example the case of a hover event in a scroll area. A naive approach involves monitoring "mouse motion in" and "mouse motion out" event, and setting your internal hover state accordingly. This is all fine and good until you have the cursor hovering over an element, and without moving the mouse, the user scrolls, suddenly throwing the hover out of sync.

It's easy enough to fix said issue, but even easier to break again because the solution isn't intuitive. It's fun to watch applications gain and lose this bug as they go through updates. The idea is to leave these tricky edge cases to the GUI framework, instead of fixing and breaking (solved) problems like this all the time while in the middle of trying to fix something completely unrelated.

The problem with individual widgets processing events is that they have no context (At least at the level that your typical GUI events exist, it is certainly possible to have higher level events, but would still require a new framework and IVD as a language has other benefits as well). In IVD, the environment is responsible for determining whether an item is hovered or not (Which was in part inspired by CSS's pseudo-classes), because it has *all* the context:

    #
    {
        color: red;

    state this.hovered:
        color: blue;

    state IVD-Mouse-Clicked:
        color: green;

    state IVD-Mouse-Clicked & this.hovered:
        color: purple; //etc
    }

Unlike CSS's pseudo-classes, IVD has a very powerful state system and allows boolean comparisons on states:

    state this.aState & (anotherState | !stateeee):

"Overlapping" attributes, where two active states define a different value for a given attribute, override in descending order:

    #
    {
        attr: one;

    state sky-is-blue:
        attr: two;

    state grass-is-green:
        attr: three;        //"three" is chosen by the runtime
    }

Models can have states:

    # -> modelname
    {
    state model.aState:
        //do something
    }

Elements can manipulate states, even in other elements:

    #anElement
    {
    state coordinatedState:
        color: blue;
    }

    #
    {
    state aState:
        induce-state: anElement.coordinatedState;

        //Can also toggle, unset, etc
        toggle-state: state;
        unset-state: state;
    }

Event-like behavior is handled by "trigger-states". These are states that are guaranteed by the runtime to only last a single frame. A bit janky conceptually, but it works:

    # -> myModel
    {
    state this.clicked:
        trigger: model.action; //Call a function defined by the model
    }


Say you have a group of states, and only one can be active at any given time. It could be tabs, or a radio-box selection, etc. IVD can manage the exclusivity for you:

    #
    {
        radio-state: one, two, three;

    state one:
    state two:
    state three:
    }

And then only one of those states will be active in that element at any given time.



## Positioning

Widget hierarchies tend to be very ridged. In traditional GUI toolkits, this is because each widget "owns" it's children. HTML isn't much better, where even if you use Javascript to restructure the DOM, it still has a specific default defined in a very different way from the dynamic position. It isn't exactly simple.

This makes it very difficult to create GUIs that are easy to re-arrange, or to exhibit dynamic, drag and drop behavior.

IVD has a completely flat, conditional positioning system. An element must choose to position itself within another element. But this decision is no more special than an element choosing it's own color based on a set of states:

    #
    {
        position-within: window;

    state this.clicked:
        position-within: anotherElement;
    }

So no position is really the "normal" one. It's all dynamic, based on the state of the system.

## Layouts/Materials

One place where traditional toolkits beat HTML/CSS I think unequivically, is in their layout system. Floats... Are unnecessarily complicated to reason about.

IVD follows a "box layout" system. The defaults are an hbox and a vbox, for horizontal and vertical rows, respectively:

    layout example

That's great, but then the order is almost arbitrary (It's actually based on the order of element declaration but bear with me). To reserve specific "slots", we have a feature called "named-cells":

    same example but named cells

Stack layout


And layouts are totally extensible. I feel like these three cover 95% of use cases, but anything more complex can be handled by a custom layout material.

## Models

A model defines a hierarchy, but elements bound to specific model items don't have to respect it too much.

The IVD philosophy is that a model shouldn't *ridgedly* define how the presentation should look. Contrast with HTML where absolutely everything is a part of the DOM in a very specific order and hierarchy, even unrelated models must be encoded in an arbitrary order.

There are two types of elements in IVD. "Free elements", which are not bound to a model and there is exactly one instance, and "enumerated elements", which are bound to a model instance, and there can be zero or more of them (one for each instance).

## Equation Solver

The thing is with a lot of stuff, you can have a nice little high level formula that clearly expresses your intent, but when you go to implement the functionality in a procedural language. It becomes very hard to understand what is going on, and much less maintain it.

    Example of equation solver



# What's Working?

The above all works (Mostly I think at least sorta probably kinda maybe borken recently I'm just one person)

- The compiler, which produces friendly error messages.
- States and state expressions.
- The layout/material system.
- Text layouts (Although laggy, see [issue](https://github.com/IrisChase/IVD/issues/2)).
- Models.
- Animations of arbitrary scalar attributes.
- The expression solver (Not tested thoroughly enough for my tastes though).

And other things too boring to list or remember.


# What is blocking alpha?

See [Alpha Release Checklist](https://github.com/IrisChase/IVD/issues/1)


# Credits

Created and developed by Iris Chase (iris@enesda.com).

# License

IVD is licensed under the terms of the Apache 2.0 license.
