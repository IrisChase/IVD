# IVD - Interactive Visual Design Language

IVD is a declarative GUI programming language and framework implementation. The goal is to make it so that you can describe *how* a interface should work and leave the rest to the computer, facilitating highly dynamic GUI programming, whilst maintaining maintainability.

# Why?

The decision to create IVD was not reached lightly. I knew it would be a huge undertaking (Although I underestimated how huge...), and spent considerable time trying to talk myself out of doing it.

I have experienced developing user interfaces in the classical way (Qt/GTK), the "modern" way (HTML/CSS/JavaScript), and played around a bit with QML, which was somewhat inspiring for this project. All of these had their pros and cons, but none of them seemed to represent a true advancement in the problem space, and I'm arrogant enough that I thought I could maybe move the needle myself.

# A Quick(ish) and Mostly Incomplete Rundown of IVD

## IVD is Not Ready

IVD is still in heavy development and as such this readme should be treated as a writeup of the project, rather than a guide on using it. It's okay if you don't get the syntax completely, just read on and the high level ought to stand out at least.

## IVD is *Not* CSS++
Before we get started, it's important to acknowledge that although the syntax is superficially similar, the theory of IVD is *very* different from the theory of CSS. Elements in IVD aren't bound to models by default, and they don't inherit attributes from their "parents" (Because they don't really have *true* parents). About the only concepts that carry over from CSS are some attribute names (Although I've renamed some that are otherwise equivalent, just to piss you off), and the box model for styling. Most everything else is either taken more from traditional GUI toolkits or is novel.

IVD allows for visual elements to be free and not bound to the model, so that your data model isn't corrupted with irrelevant noise which only exists for the presentation, as is always the case with `<DIV>` tags in any reasonably complex webpage.

CSS is for *styling* models, IVD is for *defining* complete user interfaces with [*style*](https://i.redd.it/vq2q5dqh16qy.png).

## Events Kinda Suck, State System to The Rescue
One great problem I think is the low level nature of typical GUI events. Take for example the case of an element needing to style itself differently when hovered in a scroll area. A naive approach involves monitoring "mouse motion in" and "mouse motion out" events, and setting your internal hover state accordingly. This is all fine and good until you have the cursor hovering over an element, and without moving the mouse, the user scrolls, suddenly throwing the hover out of sync.

It's easy enough to fix said issue, but even easier to break again because the solution isn't intuitive. It's fun to watch applications gain and lose this bug as they go through updates. The idea is to leave these tricky edge cases to the GUI framework, instead of fixing and breaking (solved) problems like this all the time while in the middle of trying to fix something completely unrelated.

The problem with individual widgets processing events is that they have no context (At least at the level that your typical GUI events exist, it is certainly possible to have higher level events, but IVD as a language has other benefits as well). In IVD, the environment is responsible for determining whether an item is hovered or not (Which was in part inspired by CSS's pseudo-classes):

    #element-name
    {
        color: red;

    state this.IVD-Item-Hovered:
        color: blue;

    state ::.IVD-Mouse-Clicked:
        color: green;

    state ::.IVD-Mouse-Clicked & this.IVD-Item-Hovered:
        color: purple;
    }

Unlike CSS's pseudo-classes, IVD has a very powerful state system and allows boolean comparisons on states:

    state this.aState & (anotherState | !stateeee):

"Overlapping" attributes, where two active states define a different value for a given attribute, override in descending order:

    # //anonymous element because you shouldn't be forced to name things you don't want to
    {
        attr: one;

    state sky-is-blue:
        attr: two;

    state grass-is-green:
        attr: three;        //"three" is chosen by the runtime
    }


Elements can manipulate states, even in other elements:

    #an-element
    {
    state coordinatedState:
        color: blue;
    }

    #
    {
    state aState:
        induce-state: an-element.coordinatedState;

        //Can also toggle, unset, etc
        toggle-state: state;
        unset-state: state;
    }

Event-like behavior is handled by "trigger-states". These are states that are guaranteed by the runtime to last only a single frame. This allows you to set processes external to IVD in motion using the `trigger` attribute:

    #
    {
    state this.clicked:
        trigger: IVD-Core-Quit;
    }


Say you have a group of states, and only one can be active at any given time. It could be tabs, or a radio-box selection, etc. IVD can manage the exclusivity for you:

    #
    {
        radio-state: one, two, three;

    state one:
    state two:
    state three:
    }

And then only the last state to be set will be active in that element at any given time.



## Positioning

Widget hierarchies tend to be very ridged. In traditional GUI toolkits, this is because each widget "owns" it's children, and reparenting is an arduous task. HTML isn't much better, where even if you use JavaScript to restructure the DOM, it still has a specific default defined in a very different way from how your dynamic structure is defined.

This makes it difficult to create GUIs that are easy to re-arrange.

The two points that make IVD different here are that elements are completely symmetric, and the fact that the visual element's structure isn't tightly bound to a model. When in use, a model's hierarchy only applies locally, and is largely optional (The element hierarchy doesn't necessarily have to map 1:1 to the model). [More on models below](#models).

IVD's structure being symmetric just means that positioning is always conditional. An element must choose to position itself within another element. It always starts out flat:

    #
    {
    state some-condition:
        position-within: window;

    state some-other-condition:
        position-within: another-element;
    }


## Layouts/Materials

One place where traditional toolkits beat HTML/CSS I think unequivically, is in their layout system. Floats... Are unnecessarily complicated to reason about, and enough tears have been shed over that system that I feel I need not ever speak of it again.

IVD follows a typical nested layout system. Built in layouts include hbox and vbox, for horizontal and vertical rows respectively, and the stack layout for layering.

Given the following example:

    #window
    {
        position-within: Environment; //Give us a window
        layout: hbox;
    }

    #
    {
        position-within: window;
        text: "Aye";
    }

    #
    {
        position-within: window;
        text: "Bye";
    }

    #
    {
        position-within: window;
        text: "Cye";
    }

The following is produced (please excuse the crudity of this model, I didn't have time to build it to scale or paint it):

    [AyeByeCye]

That's great, but then the order is almost arbitrary (It's actually based on the order of element declaration but bear with me). To reserve specific "slots", we have a feature called "named-cells":

    #window
    {
        position-within: Environment; //Give us a window
        layout: hbox;

        named-cells: first, middle, last;
    }

    #
    {
        position-within: window.last;
        text: "Cye";
    }

    #
    {
        position-within: window.first;
        text: "Aye";
    }

    #
    {
        position-within: window.middle;
        text: "Bye";
    }

Which produces the same output:

    [AyeByeCye]

This makes it painless to slip a column in between two elements later in the development cycle, without touching anything that already works. Empty cells are simply ignored, so they're great to declare where a notification or context popout should appear when it feels like it.

Of course, it also makes it trivial to rearrange items:

    #window
    {
        named-cells: first, middle, last;
    
    state a-state:
        named-cells: middle, first, last; //Please use better names tho
    }

Which would produce:

    [ByeAyeCye]

The built-in layouts should cover 95% (totally legit stat) of use cases simply enough. But for special cases you can extend IVD with custom materials.

## Models

A model may define a hierarchy, but elements bound to specific model items don't necessarily have to reflect it directly, and are free to position items in a root element or separate window, for example.

The IVD philosophy is that a model shouldn't *ridgedly* define how the presentation should look. Contrast with HTML where absolutely everything is a part of the DOM in a very specific order and hierarchy, even unrelated models must be encoded in an arbitrary order.

There are two types of elements in IVD. "Free elements", which are not bound to a model and there is exactly one instance, and "enumerated elements", which are bound to a model instance, and there can be zero or more of them (one for each instance).

In the previous examples, you've seen elements declared as such:

    #name-optional {}

This instantiates a single element.

Suppose you have a model uncreatively named `my-model`:

    #an-enumerated-element -> my-model {}

This creates a single instance of `an-enumerated-element` for every item in `my-model`. A "model" simply declares a list of model items. The process of binding elements to model items is known in IVD parlance as "enumeration", as elements are *enumerated* by the model.

A model item can define strings, integers, trigger slots and states. All of which may be used by elements in IVD:

    # -> model-name
    {
        text: model.the-text;

    state model.a-state:
        width: model.width-for-a-state;

    state this.clicked:
        trigger: model.react-to-click;
    }

References to the model within an element are prefixed with the keyword `model` and not the model's identifier because it allows you to easily rename the model, use generic models in classes, and because I felt like it.

Values from a model item that are used by IVD are always kept in sync. If the value changes in the model, the change is reflected in the IVD runtime.

Model states can be manipulated directly by IVD code as well:

    # -> arbitrarily-named-model-42
    {
    state x:
        induce-state: model.a-model-state;
    }

Models themselves can contain child items, allowing for complex nested data structures.

You can't position a free element within an enumerated element, you can however, position an enumerated element within a free element, or position a enumerated element within another enumerated element which share a model in common:

    #Nietzsche -> model-name;

    # -> model-name
    {
        position-within: Nietzsche;
    }

The runtime will find the correct instance of `Nietzsche` to position the anonymous element within.

This actually works with any common ancestor, suppose the following:

    #an-elephant -> elephants
    {
        layout: vbox;
    }

    #leg -> elephants::legs
    {
        position-within: an-elephant;
    }

This allows you to create an element for each collection of an item, and allow it to contain each child instance of said collection.

Suppose you want ordered items (As one often does). Perhaps the model has triggers defined for sorting the model according to different criteria. The order of elements in IVD can be bound to the model order:

    #an-elephant -> elephants
    {
        layout: vbox;
        model-order: enable; //Sort child elements according to model
    }

    #leg -> elephants::legs
    {
        position-within: an-elephant;
    }

The plan is to be able to rearrange items in IVD, and then have the new order backpropogated to the model as well.

## Equation Solver

IVD allows you to define scalar constraints as equations which are kept up-to-date automatically:

    #element1
    {
        width: other-element.height * 2;
    }

The trouble with the above, is that it isn't flexible. What if you just really wanted for `element1.width == 200` to be true, but without violating the constraint?

    #element2
    {
    state I-want-to-set-something:
        set: element1 = 200; //error because it doesn't know what to do
    }

Wouldn't it be nice if you could get IVD to figure out what `other-element.height` needs to be in order for `element1.width` to equal 200?

    #element1
    {
        width: [other-element.height] * 2; //Declare which value in the expression is weak
    }

    #element2
    {
    state I-want-to-set-something:
        set: element1 = 200; //Works, because now it knows to solve for other-element.height
    }


What happens in the above is that the expression in `element1.width` is solved for `other-element.height`, and the result is backpropogated to `other-element.height` (Which may be defined as a variable or an expression with a weak value, it can be turtles all the way down). Once that is updated, the expression in `element1.width` is reevaluated.

It's important to think of this as more of a suggestion than an absolute order. `other-element.height` might have a min/max constraint which rounds off the value being propogated, and then ***that*** value is what is observed when `element1.width` is reevaluated. Nothing is ever left in an inconsistent state.

Scalars declared by a model can be back-propogated to as well:


    #element1 -> my-model
    {
        width: [model.a-val] * 2;
    }

Everything always obeys the constraints as defined, but you also have the power to update the observed values arbitrarily, giving you the best of both worlds.

## Classes

Classes are very simple. They're really just templates from which attributes are copied:

    .class-name
    {
        color: blue;
    }

    # : class-name
    {
        //color is blue
    }

Shorthand version if you don't need to declare anything in the body of the deriving element:

    # : class-name;

An element can derive from an arbitrary number of classes, and the attributes are overriden in the order that the classes are declared:

    .another-class
    {
        color: yellow;

    state ::.IVD-Mouse-Motion:
        color: green;
    }

    # : another-class, class-name
    {
        //color is blue

    state ::.IVD-Mouse-Motion:
        //color is green
    }

    # : class-name, another-class //Class list order different
    {
        //color is yellow this time

    state ::.IVD-Mouse-Motion:
        //color is still green because it's not in conflict
    }

## Remorial Classes for Code Reuse

And last but *certainly* not least.

Suppose you have the following construct:

    #contextual-dialog
    {
        layout: vbox;
        named-cells: message-cell, input-cell, confirmation-cell;
    }

    #message-area
    {
        position-within: contextual-dialog.message-cell;
        layout: hbox;
        named-cells: image-cell, text-cell;
    }

    #message-image
    {
        position-within: message-area.image-cell;
        image: "image-uri";
    }

    #message-text
    {
        position-within: message-area.text-cell;
        text: "Enter info below";
    }

    //etc just use your imagination for the confirmation cell

Everything is fine and good until... You need to reuse that. You can't simply use a class because a class only helps with a single element, and the above can only work with several elements.

Remorial classes are a way of defining a "composite" element. You define the class as normal, and then attach "remoras" (get it?) which are just a special kind of element to it. Whenever an element derives from this class, a copy of each remora is also spun up as well, facilitating reuse of complex elements.

And the syntax is quite simple:

    .contextual-dialog
    {
        layout: vbox;
        named-cells: message-cell, input-cell, confirmation-cell;
    }

    @contextual-dialog.message-area
    {
        position-within: @.message-cell;
        layout: hbox;
        named-cells: image-cell, text-cell;
    }

    @conceptual-dialog.message-image
    {
        position-within: @::message-area.image-cell;
        image: "image-uri";
    }

    @conceptual-dialog.message-text
    {
        position-within: @::message-area.text-cell;
        text: "Enter info below";
    }

    //etc

    # : contextual-dialog;

This is equivalent to the previous example, except that it is reusable, of course.

Where the remora operator (`@`) is used within an element, it is substituted by the actual instance name given to the element which derives from the remorial class. In the above examples, this name is auto-generated as the elements are anonymous.

One special feature of remora substitution is that you can address any remora in the "school" (get it???) from any other element in the school using the `@::element-name` syntax, which is substituted with the name generated for it by the compiler. [This allows for a remorial class to export values from a child remora](https://github.com/IrisChase/IVD/blob/11066e421ff74b9418b420a3890e5d9fdcf40f6e/src/tests/valid/remoravaluekeysubstitution.ivd#L19) (Which are all otherwise unaddressable), but the implications of this are outside the scope of this little hoe-down.

The remora example above is obviously incomplete. The biggest failing is that it really should be bound to a model in order to have a place to actually send input data and triggers for buttons. 
Remoras work with nested models, and common parent deduction and all that good stuff as well. They're just an additional type of template which tag alongside otherwise normal classes.

And of course remoras can be nested but an example that *proves* as much is a bear to read (or write) so I'll spare you (and myself).

Again, remoras are just syntactic sugar, they are expanded by the compiler. The resulting elements are exactly the same as if they had been defined manually. A little bit of witchcraft and maybe some symbol substitution makes it all come together quite nicely~


## And Other Stuff Probably

This is by no means a complete overview of the features developed or in development for IVD. We haven't even mentioned the (working!) animation system or the ability to declare expressions (which is to allow for complex widget interactions such as scrollbars or sliders affecting viewports, all defined within IVD), or the [C](https://github.com/IrisChase/IVD/blob/development/src/user_include/IVD_c.h) and [C++](https://github.com/IrisChase/IVD/blob/development/src/user_include/IVD_cpp.h) bindings... It is simply meant to give you a taste for the project.

# What's Working?

In no particular order:

- The compiler, which produces friendly error messages.
- Remorial class instantiation.
- Basic element styling.
- States and state expressions.
- The layout/material system.
- Text layouts (Although laggy, see [issue](https://github.com/IrisChase/IVD/issues/2)).
- Models, enumeration, common ancestor deduction, etc.
- Animations of arbitrary scalar attributes.
- The equation solver (Not tested thoroughly enough for my tastes though).
- And a bunch of other stuff, probably.



# What is blocking alpha?

See [Alpha Release Checklist](https://github.com/IrisChase/IVD/issues/1)

# Contributing

\*sounds of deranged cackling echo in the distance\*

# Credits

Created and developed by Iris Chase (iris@enesda.com).

# License

IVD is licensed under the terms of the Apache 2.0 license.
