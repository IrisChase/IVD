# IVD - Interactive Visual Design Language

IVD is a declarative GUI programming language and framework implementation. The goal is to make it so that you can describe *how* a interface should work and leave the rest to the computer, facilitating highly dynamic GUI programming, whilst maintaining maintainability.

# Why?

The decision to create IVD was not reached lightly. I knew it would be a huge undertaking (Although I underestimated how huge), and spent considerable time trying to talk myself out of doing it.

I have experienced developing user interfaces in the classical way (Qt/GTK), the "modern" way (HTML/CSS/Javascript), and played around a bit with QML, which was somewhat inspiring for this project. All of these had their pros and cons, but none of them seemed to get at the heart of the problem.

# A Quick(ish) and Mostly Incomplete Rundown of IVD
## Events Kinda Suck, State System to The Rescue
One great problem I think is the low level nature of typical GUI events. Take for example the case of an element needing to style itself differently when hovered in a scroll area. A naive approach involves monitoring "mouse motion in" and "mouse motion out" event, and setting your internal hover state accordingly. This is all fine and good until you have the cursor hovering over an element, and without moving the mouse, the user scrolls, suddenly throwing the hover out of sync.

It's easy enough to fix said issue, but even easier to break again because the solution isn't intuitive. It's fun to watch applications gain and lose this bug as they go through updates. The idea is to leave these tricky edge cases to the GUI framework, instead of fixing and breaking (solved) problems like this all the time while in the middle of trying to fix something completely unrelated.

The problem with individual widgets processing events is that they have no context (At least at the level that your typical GUI events exist, it is certainly possible to have higher level events, but would still require a new framework and IVD as a language has other benefits as well). In IVD, the environment is responsible for determining whether an item is hovered or not (Which was in part inspired by CSS's pseudo-classes):

    #elementName
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

    # //anonymous element
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

Event-like behavior is handled by "trigger-states". These are states that are guaranteed by the runtime to last only a single frame. A bit janky conceptually, but it works:

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

Widget hierarchies tend to be very ridged. In traditional GUI toolkits, this is because each widget "owns" it's children, and reparenting is an arduous task. HTML isn't much better, where even if you use Javascript to restructure the DOM, it still has a specific default defined in a very different way from the dynamic structure.

This makes it difficult to create GUIs that are easy to re-arrange, or to exhibit dynamic, drag and drop behavior.

Part of what makes IVD different here is the fact that the visual element's structure is only loosely bound to the model. The model's structure is essentially ridgid, but it only enforces it's hierarchy locally; that is, the hierarchy is fixed, but it can be placed anywhere. So you get the power of binding the structure of the layout to a model, but with the flexibility to freely position the elements according to the visual theme, which need not be encoded in the model. ([More on models below](#models)).

IVD has a completely dynamic, conditional positioning system. An element must choose to position itself within another element. There is no "default" hierarchy:

    #someElement
    {
    state someCondition:
        position-within: window;

    state someOtherCondition:
        position-within: anotherElement;
    }

So no position is really the "normal" one. It's all dynamic, based on the state of the system, and largely independent of the model structure, except for when and where you want it to mirror the model.

## Layouts/Materials

One place where traditional toolkits beat HTML/CSS I think unequivically, is in their layout system. Floats... Are unnecessarily complicated to reason about.

IVD follows a typical nested layout system. Built in layouts include hbox and vbox, for horizontal and vertical rows respectively, and the stack layout which covers layering elements.

    layout example

That's great, but then the order is almost arbitrary (It's actually based on the order of element declaration but bear with me). To reserve specific "slots", we have a feature called "named-cells":

    same example but named cells


And layouts are totally extensible. I feel like these three cover 95% of use cases, but anything more complex can be handled by a custom layout material.

## Models

A model defines a hierarchy, but elements bound to specific model items don't have to respect it too much.

The IVD philosophy is that a model shouldn't *ridgedly* define how the presentation should look. Contrast with HTML where absolutely everything is a part of the DOM in a very specific order and hierarchy, even unrelated models must be encoded in an arbitrary order.

There are two types of elements in IVD. "Free elements", which are not bound to a model and there is exactly one instance, and "enumerated elements", which are bound to a model instance, and there can be zero or more of them (one for each instance).

## Equation Solver

IVD allows you to define scalar constraints as equations which are kept up-to-date automatically:

    #element1
    {
        width: otherElement.height * 2;
    }

The trouble with the above, is that you can't set it.

    #element2
    {
    state I-want-to-set-something:
        set: element1 = 200; //error because it doesn't know what to do
    }

Wouldn't it be nice if you could get IVD to figure out what `otherElement.height` needs to be in order for `element1.width` to equal 200?

    #element1
    {
        width: [otherElement.height] * 2; //Declare which value in the expression is weak
    }

    #element2
    {
    state I-want-to-set-something:
        set: element1 = 200; //Works, because now it knows to solve for otherElement.height
    }


What happens in the above is that the expression in `element1.width` is solved for `otherElement.height`, and the result is backpropogated to `otherElement.height` (Which may also be defined as an expression with a weak value, it can be turtles all the way down). Once that is updated, the expression in `element1.width` is reevaluated.

It's important to think of this as more of a suggestion than an absolute order. `otherElement.height` might have a min/max constraint which rounds off the value being propogated, and then THAT value is what is observed when `element1.width` is reevaluated. Nothing is ever left in an inconsistent state.

Everything always obeys the constraints as defined, but you also have the power to update the observed values arbitrarily, giving you the best of both worlds.

# What's Working?

The above all works (Mostly)

- The compiler, which produces friendly error messages.
- Basic element styling.
- States and state expressions.
- The layout/material system.
- Text layouts (Although laggy, see [issue](https://github.com/IrisChase/IVD/issues/2)).
- Models.
- Animations of arbitrary scalar attributes.
- The expression solver (Not tested thoroughly enough for my tastes though).

And other things too boring or obvious to list or remember.


# What is blocking alpha?

See [Alpha Release Checklist](https://github.com/IrisChase/IVD/issues/1)


# Credits

Created and developed by Iris Chase (iris@enesda.com).

# License

IVD is licensed under the terms of the Apache 2.0 license.
